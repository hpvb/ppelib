#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "pelib-header.h"
#include "pelib-section.h"
#include "utils.h"
#include "constants.h"

typedef struct data_directory {
	pelib_section_t* section;
	uint32_t offset;
	uint32_t size;

	uint32_t orig_rva;
	uint32_t orig_size;
} data_directory_t;

typedef struct pefile {
        size_t pe_header_offset;
        size_t coff_header_offset;
	size_t section_offset;
        size_t end_of_sections;

       	pelib_header_t header;
	pelib_section_t** sections;
	data_directory_t* data_directories;

        uint8_t* stub;
	size_t trailing_data_size;
	uint8_t* trailing_data;
} pefile_t;

int read_pe_file(const char* filename, uint8_t** file, size_t* size, uint32_t* pe_header_offset) {
	FILE *f = fopen(filename, "r");

	if (fseek(f, PE_SIGNATURE, SEEK_SET) == -1) {
		perror("Seeking to PE header offset");
		return 1;
	}

	size_t retsize;

	retsize = fread(pe_header_offset, 1, 4, f);
	if (retsize != 4) {
		fprintf(stderr, "Couldn't read PE header offset. Got %li bytes, expected 4\n", retsize);
		return 1;
	}

	if (fseek(f, *pe_header_offset, SEEK_SET) == -1) {
		perror("Seeking to PE header");
		return 1;
	}

	uint8_t signature[4];
	retsize = fread(&signature, 1, 4, f);
	if (retsize != 4) {
		fprintf(stderr, "Couldn't read PE signature. Got %li bytes, expected 4\n", retsize);
		return 1;
	}

	if (memcmp(signature, "PE\0", 4) != 0) {
		fprintf(stderr, "Not a PE file. Got 0x%X 0x%X 0x%X 0x%X, expected 0x%X 0x%X 0x%X 0x%X\n", signature[0], signature[1], signature[2], signature[3], 'P', 'E', 0, 0);
	}

	fseek(f, 0, SEEK_END);
	*size = ftell(f);
	rewind(f);
	
	*file = malloc(*size);
	if (! *file) {
		fprintf(stderr, "Unable to allocate memory\n");
		return 1;
	}

	retsize = fread(*file, 1, *size, f);
	if (retsize != *size) {
		fprintf(stderr, "Couldn't file. Got %li bytes, expected 4\n", retsize);
		return 1;
	}
	fclose(f);

	return 0;
}

int write_pe_file(const char* filename, const pefile_t* pe) {
	uint8_t* buffer = NULL;
	size_t size = 0;
	size_t write = 0;

	// Write stub
	size += pe->pe_header_offset;
	size += 4;
	size_t coff_header_size = serialize_pe_header(&pe->header, NULL, pe->pe_header_offset);
	size += coff_header_size;
	size_t end_of_sections = 0;

	size_t section_offset = pe->pe_header_offset + coff_header_size;
	for (uint32_t i = 0; i < pe->header.number_of_sections; ++i) {
		size_t section_size = serialize_section(pe->sections[i], NULL, section_offset + (i * PE_SECTION_HEADER_SIZE));

		if (section_size > end_of_sections) {
			end_of_sections = section_size;
		}
	}

	// Theoretically all the sections could be before the header
	if (end_of_sections > size) {
		size = end_of_sections;
	}

	size += pe->trailing_data_size;

	printf("Size of coff_header        : %li\n", coff_header_size);
	printf("Size of sections           : %li\n", end_of_sections);
	printf("Size of trailing data      : %li\n", pe->trailing_data_size);
	printf("Total size                 : %li\n", size);

	buffer = realloc(buffer, size);
	if (! buffer) {
		fprintf(stderr, "Failed to allocate\n");
		return 1;
	}
	memset(buffer, 0, size);
	//memset(buffer, 0xFF, size);

	memcpy(buffer, pe->stub, pe->pe_header_offset);
	write += pe->pe_header_offset;

	// Write PE header
	memcpy(buffer + write, "PE\0", 4);
	write += 4;

	// Write COFF header
	serialize_pe_header(&pe->header, buffer, pe->pe_header_offset + 4);

	// Write sections
	for (uint32_t i = 0; i < pe->header.number_of_sections; ++i) {
		serialize_section(pe->sections[i], buffer, section_offset + 4 + (i * PE_SECTION_HEADER_SIZE));
	}

	// Write trailing data
	memcpy(buffer + end_of_sections, pe->trailing_data, pe->trailing_data_size);
	
	FILE *f = fopen(filename, "w+");
	fwrite(buffer, 1, size, f);

	fclose(f);
	free(buffer);

	return size;
}

void recalculate(pefile_t* pe) {
	size_t coff_header_size = serialize_pe_header(&pe->header, NULL, pe->pe_header_offset);
	size_t size_of_headers = pe->pe_header_offset + 4 + coff_header_size + (pe->header.number_of_sections * PE_SECTION_HEADER_SIZE);
	pe->header.size_of_headers = TO_NEAREST(size_of_headers, pe->header.file_alignment);

	size_t next_section_virtual = 0x1000;
	size_t next_section_physical = pe->header.size_of_headers;
	uint32_t base_of_code = 0;
	uint32_t base_of_data = 0;

	for (uint32_t i = 0; i < pe->header.number_of_sections; ++i) {
		if (pe->sections[i]->size_of_raw_data && pe->sections[i]->virtual_size <= pe->sections[i]->size_of_raw_data) {
			pe->sections[i]->size_of_raw_data = TO_NEAREST(pe->sections[i]->virtual_size, pe->header.file_alignment);
		}

		pe->sections[i]->virtual_address = next_section_virtual;

		if (pe->sections[i]->size_of_raw_data) {
			pe->sections[i]->pointer_to_raw_data = next_section_physical;
		}

		next_section_virtual = TO_NEAREST(pe->sections[i]->virtual_size, pe->header.section_alignment) + next_section_virtual;
		next_section_physical = TO_NEAREST(pe->sections[i]->size_of_raw_data, pe->header.file_alignment) + next_section_physical;

		if (! base_of_code && CHECK_BIT(pe->sections[i]->characteristics, IMAGE_SCN_CNT_CODE)) {
			base_of_code = pe->sections[i]->virtual_address;
		}

		if (! base_of_data && ! CHECK_BIT(pe->sections[i]->characteristics, IMAGE_SCN_CNT_CODE)) {
			base_of_data = pe->sections[i]->virtual_address;
		}
	}

	pe->header.base_of_code = base_of_code;
	pe->header.base_of_data = base_of_data;

	size_t virtual_sections_end = pe->sections[pe->header.number_of_sections - 1]->virtual_address + pe->sections[pe->header.number_of_sections - 1]->virtual_size;
	pe->header.size_of_image = TO_NEAREST(virtual_sections_end, pe->header.section_alignment);

	for (uint32_t i = 0; i < pe->header.number_of_rva_and_sizes; ++i) {
		if (! pe->data_directories[i].section) {
			pe->header.data_directories[i].virtual_address = 0;
			pe->header.data_directories[i].size = 0;
		} else {
			uint32_t directory_va = pe->data_directories[i].section->virtual_address + pe->data_directories[i].offset;
			uint32_t directory_size = pe->data_directories[i].size;

			pe->header.data_directories[i].virtual_address = directory_va;
			pe->header.data_directories[i].size = directory_size;
		}
	}
}

int main(int argc, char* argv[]) {
	if (! argc) return 1;

	pefile_t pe = {0};
	uint8_t* file;
	size_t size;
	uint32_t pe_header_offset;

	if (read_pe_file(argv[1], &file, &size, &pe_header_offset)) {
		return 1;
	}

	pe.pe_header_offset = pe_header_offset;
	pe.coff_header_offset = pe_header_offset + 4;

	if (size < pe.coff_header_offset + COFF_HEADER_SIZE) {
		fprintf(stderr, "File size too small\n");
		return 1;
	}

	size_t header_size = deserialize_pe_header(file, pe.coff_header_offset, size, &pe.header);
	pe.section_offset = header_size + pe.coff_header_offset;

	print_pe_header(&pe.header);
	printf("\n");

	pe.sections = malloc(sizeof(pelib_section_t*) * pe.header.number_of_sections);
	pe.data_directories = calloc(sizeof(data_directory_t) * pe.header.number_of_rva_and_sizes, 1);
	pe.end_of_sections = 0;

	for (uint32_t i = 0; i < pe.header.number_of_sections; ++i) {
		pe.sections[i] = malloc(sizeof(pelib_section_t));
		size_t section_size = deserialize_section(file, pe.section_offset + (i * PE_SECTION_HEADER_SIZE), size, pe.sections[i]);

		if (section_size > pe.end_of_sections) {
			pe.end_of_sections = section_size;
		}

		for (uint32_t d = 0; d < pe.header.number_of_rva_and_sizes; ++d) {
			size_t directory_va = pe.header.data_directories[d].virtual_address;
			size_t directory_size = pe.header.data_directories[d].size;
			size_t directory_end = pe.header.data_directories[d].virtual_address + pe.header.data_directories[d].size;
			size_t section_va = pe.sections[i]->virtual_address;
			size_t section_end = section_va + pe.sections[i]->size_of_raw_data;

			if (section_va <= directory_va && section_end >= directory_end) {
				pe.data_directories[d].section = pe.sections[i];
				pe.data_directories[d].offset = directory_va - section_va;
				pe.data_directories[d].size = directory_size;
				pe.data_directories[d].orig_rva = directory_va;
				pe.data_directories[d].orig_size = directory_size;
			}
		}
		print_section(pe.sections[i]);
		printf("\n");
	}

	//void* t = pe.sections[4];
	//pe.sections[4] = pe.sections[3];
	//pe.sections[3] = t;

        pe.stub = malloc(pe.pe_header_offset);
        if (! pe.stub) {
                fprintf(stderr, "Failed to allocate memory\n");
                return 1;
        }
        memcpy(pe.stub, file, pe.pe_header_offset);

        if (size > pe.end_of_sections) {
                pe.trailing_data_size = size - pe.end_of_sections;
                pe.trailing_data = malloc(pe.trailing_data_size);

                memcpy(pe.trailing_data, file + pe.end_of_sections, pe.trailing_data_size);
        }

	recalculate(&pe);
	write_pe_file("out.exe", &pe);

	free(file);
	free(pe.stub);
	for (uint32_t i = 0; i < pe.header.number_of_sections; ++i) {
		free(pe.sections[i]->contents);
		free(pe.sections[i]);
	}
	free(pe.data_directories);
	free(pe.header.data_directories);
	free(pe.sections);
	free(pe.trailing_data);
}
