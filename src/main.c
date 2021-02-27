#include <errno.h>
#include <pelib/pelib-constants.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "pelib-error.h"
#include "pelib-generated.h"
#include "main.h"

pelib_file_t* pelib_create() {
	pelib_new_error = NULL;

	pelib_file_t *pe = calloc(sizeof(pelib_file_t), 1);
	if (!pe) {
		pelib_set_error("Failed to allocate PE structure");
	}
	return pe;
}

void pelib_destroy(pelib_file_t *pe) {
	if (!pe) {
		return;
	}

	free(pe->stub);
	for (size_t i = 0; i < pe->header.number_of_sections; ++i) {
		free(pe->sections[i]->contents);
		free(pe->sections[i]);
	}
	for (size_t i = 0; i < pe->certificate_table.size; ++i) {
		free(pe->certificate_table.certificates[i].certificate);
	}
	free(pe->certificate_table.certificates);
	free(pe->data_directories);
	free(pe->header.data_directories);
	free(pe->sections);
	free(pe->trailing_data);
	free(pe->file_contents);

	free(pe);
}

pelib_file_t* pelib_create_from_buffer(uint8_t *buffer, size_t size) {
	pelib_new_error = NULL;

	if (size < PE_SIGNATURE_OFFSET + sizeof(uint32_t)) {
		pelib_set_error("Not a PE file (file too small)");
		free(buffer);
		return NULL;
	}

	uint32_t header_offset = read_uint32_t(buffer + PE_SIGNATURE_OFFSET);
	if (size < header_offset + sizeof(uint32_t)) {
		pelib_set_error("Not a PE file (file too small for PE signature)");
		free(buffer);
		return NULL;
	}

	uint32_t signature = read_uint32_t(buffer + header_offset);
	if (signature != PE_SIGNATURE) {
		pelib_set_error("Not a PE file (PE00 signature missing)");
		free(buffer);
		return NULL;
	}

	pelib_file_t *pe = pelib_create();
	if (pelib_error_peek()) {
		pelib_destroy(pe);
		return NULL;
	}

	pe->file_size = size;
	pe->file_contents = buffer;
	pe->pe_header_offset = header_offset;
	pe->coff_header_offset = header_offset + 4;

	if (size < pe->coff_header_offset + COFF_HEADER_SIZE) {
		pelib_set_error("Not a PE file (file too small for COFF header)");
		pelib_destroy(pe);
		return NULL;
	}

	size_t header_size = deserialize_pe_header(pe->file_contents, pe->coff_header_offset, pe->file_size, &pe->header);
	if (pelib_error_peek()) {
		pelib_destroy(pe);
		return NULL;
	}

	pe->section_offset = header_size + pe->coff_header_offset;
	pe->sections = malloc(sizeof(pelib_section_t*) * pe->header.number_of_sections);
	if (!pe->sections) {
		pelib_set_error("Failed to allocate sections");
		pelib_destroy(pe);
		return NULL;
	}

	pe->data_directories = calloc(sizeof(data_directory_t) * pe->header.number_of_rva_and_sizes, 1);
	if (!pe->data_directories) {
		pelib_set_error("Failed to allocate data directories");
		pelib_destroy(pe);
		return NULL;
	}

	pe->end_of_sections = 0;

	for (uint32_t i = 0; i < pe->header.number_of_sections; ++i) {
		pe->sections[i] = malloc(sizeof(pelib_section_t));
		if (!pe->sections[i]) {
			pelib_set_error("Failed to allocate section");
			pelib_destroy(pe);
			return NULL;
		}

		size_t section_size = deserialize_section(pe->file_contents, pe->section_offset + (i * PE_SECTION_HEADER_SIZE),
				pe->file_size, pe->sections[i]);

		if (pelib_error_peek()) {
			pelib_destroy(pe);
			return NULL;
		}

		if (section_size > pe->end_of_sections) {
			pe->end_of_sections = section_size;
		}

		for (uint32_t d = 0; d < pe->header.number_of_rva_and_sizes; ++d) {
			size_t directory_va = pe->header.data_directories[d].virtual_address;
			size_t directory_size = pe->header.data_directories[d].size;
			size_t section_va = pe->sections[i]->virtual_address;
			size_t section_end = section_va + pe->sections[i]->size_of_raw_data;

			if (section_va <= directory_va) {
				//printf("Considering directory %s in section %s\n", data_directory_names[d], pe.sections[i]->name);
				if (section_end >= directory_va) {
					pe->data_directories[d].section = pe->sections[i];
					pe->data_directories[d].offset = directory_va - section_va;
					pe->data_directories[d].size = directory_size;
					pe->data_directories[d].orig_rva = directory_va;
					pe->data_directories[d].orig_size = directory_size;

					//printf("Found directory %s in section %s\n", data_directory_names[d], pe.sections[i]->name);
				} else {
					//printf("Directory %s doesn't fit in section %s. 0x%08lx < 0x%08lx\n", data_directory_names[d], pe.sections[i]->name, section_end, directory_va);
				}
			}
		}
	}

	if (pe->header.data_directories[DIR_CERTIFICATE_TABLE].size) {
		deserialize_certificate_table(pe->file_contents, &pe->header, pe->file_size, &pe->certificate_table);
		if (pelib_error_peek()) {
			pelib_destroy(pe);
			return NULL;
		}
	}

	pe->start_of_sections = pe->sections[0]->virtual_address;

	//void* t = pe.sections[4];
	//pe.sections[4] = pe.sections[3];
	//pe.sections[3] = t;

	pe->stub = malloc(pe->pe_header_offset);
	if (!pe->stub) {
		pelib_set_error("Failed to allocate memory for PE stub");
		pelib_destroy(pe);
		return NULL;
	}
	memcpy(pe->stub, pe->file_contents, pe->pe_header_offset);

	if (size > pe->end_of_sections) {
		pe->trailing_data_size = size - pe->end_of_sections;
		pe->trailing_data = malloc(pe->trailing_data_size);

		if (!pe->trailing_data) {
			pelib_set_error("Failed to allocate memory for trailing data");
			pelib_destroy(pe);
			return NULL;
		}

		memcpy(pe->trailing_data, pe->file_contents + pe->end_of_sections, pe->trailing_data_size);
	}

	return pe;
}

pelib_file_t* pelib_create_from_file(const char *filename) {
	pelib_new_error = NULL;
	size_t file_size;
	uint8_t *file_contents;

	FILE *f = fopen(filename, "rb");

	if (!f) {
		pelib_set_error("Failed to open file");
		return NULL;
	}

	fseek(f, 0, SEEK_END);
	file_size = ftell(f);
	rewind(f);

	file_contents = malloc(file_size);
	if (!file_size) {
		fclose(f);
		pelib_set_error("Failed to allocate file data");
		return NULL;
	}

	size_t retsize = fread(file_contents, 1, file_size, f);
	if (retsize != file_size) {
		fclose(f);
		pelib_set_error("Failed to read file data");
		return NULL;
	}

	fclose(f);

	return pelib_create_from_buffer(file_contents, file_size);
}

size_t pelib_write_to_buffer(pelib_file_t *file, uint8_t *buffer, size_t size) {
	pelib_new_error = NULL;

}

size_t pelib_write_to_file(pelib_file_t *file, const char *filename) {
	pelib_new_error = NULL;

}

pelib_header_t* pelib_get_header(pelib_file_t *pe) {
	pelib_new_error = NULL;

	pelib_header_t *retval = malloc(sizeof(pelib_header_t));
	if (!retval) {
		pelib_set_error("Unable to allocate header");
		return NULL;
	}

	memcpy(retval, &pe->header, sizeof(pelib_header_t));
	return retval;
}

void pelib_set_header(pelib_file_t *pe, pelib_header_t *header) {
	pelib_new_error = NULL;

	if (header->magic != PE32_MAGIC || header->magic != PE32PLUS_MAGIC) {
		pelib_set_error("Unknown magic");
		return;
	}

	if (header->number_of_sections != pe->header.number_of_sections) {
		pelib_set_error("number_of_sections mismatch");
		return;
	}

	if (header->number_of_rva_and_sizes != pe->header.number_of_rva_and_sizes) {
		pelib_set_error("number_of_rva_and_sizes mismatch");
	}

	if (header->size_of_headers != pe->header.size_of_headers) {
		pelib_set_error("size_of_headers mismatch");
	}

	memcpy(&pe->header, header, sizeof(pelib_header_t));
}

void pelib_free_header(pelib_header_t *header) {
	free(header);
}

int write_pe_file(const char *filename, const pelib_file_t *pe) {
	uint8_t *buffer = NULL;
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

	size_t certificates_size = serialize_certificate_table(&pe->certificate_table, NULL);

	if (certificates_size > size) {
		size = certificates_size;
	}

	printf("Size of coff_header        : %li\n", coff_header_size);
	printf("Size of sections           : %li\n", end_of_sections);
	printf("Size of trailing data      : %li\n", pe->trailing_data_size);
	printf("Total size                 : %li\n", size);

	buffer = realloc(buffer, size);
	if (!buffer) {
		fprintf(stderr, "Failed to allocate\n");
		return 1;
	}
	memset(buffer, 0, size);
	//memset(buffer, 0xCC, size);

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

	// Write certificates
	serialize_certificate_table(&pe->certificate_table, buffer);

	FILE *f = fopen(filename, "w+");
	fwrite(buffer, 1, size, f);

	fclose(f);
	free(buffer);

	return size;
}

void pelib_recalculate(pelib_file_t *pe) {
	size_t coff_header_size = serialize_pe_header(&pe->header, NULL, pe->pe_header_offset);
	size_t size_of_headers = pe->pe_header_offset + 4 + coff_header_size
			+ (pe->header.number_of_sections * PE_SECTION_HEADER_SIZE);

	size_t next_section_virtual = pe->start_of_sections;
	size_t next_section_physical = pe->header.size_of_headers;

	uint32_t base_of_code = 0;
	uint32_t base_of_data = 0;
	uint32_t size_of_initialized_data = 0;
	uint32_t size_of_uninitialized_data = 0;
	uint32_t size_of_code = 0;

	for (uint32_t i = 0; i < pe->header.number_of_sections; ++i) {
		pelib_section_t *section = pe->sections[i];

		if (section->size_of_raw_data && section->virtual_size <= section->size_of_raw_data) {
			section->size_of_raw_data = TO_NEAREST(section->virtual_size, pe->header.file_alignment);
		}

		section->virtual_address = next_section_virtual;

		if (section->size_of_raw_data) {
			section->pointer_to_raw_data = next_section_physical;
		}

		next_section_virtual =
		TO_NEAREST(section->virtual_size, pe->header.section_alignment) + next_section_virtual;
		next_section_physical =
		TO_NEAREST(section->size_of_raw_data, pe->header.file_alignment) + next_section_physical;

		if (CHECK_BIT(section->characteristics, IMAGE_SCN_CNT_CODE)) {
			if (!base_of_code) {
				base_of_code = section->virtual_address;
			}

			// This appears to hold empirically true.
			if (strcmp(".bind", section->name) != 0) {
				size_of_code += TO_NEAREST(section->virtual_size, pe->header.file_alignment);
			}
		}

		if (!base_of_data && !CHECK_BIT(section->characteristics, IMAGE_SCN_CNT_CODE)) {
			base_of_data = section->virtual_address;
		}

		if (CHECK_BIT(section->characteristics, IMAGE_SCN_CNT_INITIALIZED_DATA)) {
			// This appears to hold empirically true.
			if (pe->header.magic == PE32_MAGIC) {
				uint32_t vs = TO_NEAREST(section->virtual_size, pe->header.file_alignment);
				uint32_t rs = section->size_of_raw_data;
				size_of_initialized_data += MAX(vs, rs);
			} else if (pe->header.magic == PE32PLUS_MAGIC) {
				size_of_initialized_data += TO_NEAREST(section->size_of_raw_data, pe->header.file_alignment);
			}
		}

		if (CHECK_BIT(section->characteristics, IMAGE_SCN_CNT_UNINITIALIZED_DATA)) {
			size_of_uninitialized_data += TO_NEAREST(section->virtual_size, pe->header.file_alignment);
		}
	}

	pe->header.base_of_code = base_of_code;
	// The actual value of these of PE images in the wild varies a lot.
	// There doesn't appear to be an actual correct way of calculating these

	pe->header.base_of_data = base_of_data;

	pe->header.size_of_initialized_data = TO_NEAREST(size_of_initialized_data, pe->header.file_alignment);
	pe->header.size_of_uninitialized_data = TO_NEAREST(size_of_uninitialized_data, pe->header.file_alignment);
	pe->header.size_of_code = TO_NEAREST(size_of_code, pe->header.file_alignment);

	size_t virtual_sections_end = pe->sections[pe->header.number_of_sections - 1]->virtual_address
			+ pe->sections[pe->header.number_of_sections - 1]->virtual_size;
	pe->header.size_of_image = TO_NEAREST(virtual_sections_end, pe->header.section_alignment);

	pe->header.size_of_headers = TO_NEAREST(size_of_headers, pe->header.file_alignment);

	for (uint32_t i = 0; i < pe->header.number_of_rva_and_sizes; ++i) {
		if (!pe->data_directories[i].section) {
			pe->header.data_directories[i].virtual_address = 0;
			pe->header.data_directories[i].size = 0;
		} else {
			uint32_t directory_va = pe->data_directories[i].section->virtual_address + pe->data_directories[i].offset;
			uint32_t directory_size = pe->data_directories[i].size;

			pe->header.data_directories[i].virtual_address = directory_va;
			pe->header.data_directories[i].size = directory_size;
		}
	}

	if (pe->certificate_table.size) {
		size_t size = 0;
		for (uint32_t i = 0; i < pe->certificate_table.size; ++i) {
			size += pe->certificate_table.certificates[i].length;
		}

		pe->header.data_directories[DIR_CERTIFICATE_TABLE].virtual_address = pe->certificate_table.offset;
		pe->header.data_directories[DIR_CERTIFICATE_TABLE].size = size;
	}
}

#if 0
int main(int argc, char *argv[]) {
	if (!argc)
		return 1;

	pelib_file_t pe = { 0 };
	uint8_t *file;
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
		size_t section_size = deserialize_section(file, pe.section_offset + (i * PE_SECTION_HEADER_SIZE), size,
				pe.sections[i]);

		if (section_size > pe.end_of_sections) {
			pe.end_of_sections = section_size;
		}

		for (uint32_t d = 0; d < pe.header.number_of_rva_and_sizes; ++d) {
			size_t directory_va = pe.header.data_directories[d].virtual_address;
			size_t directory_size = pe.header.data_directories[d].size;
			size_t directory_end = directory_va + directory_size;
			size_t section_va = pe.sections[i]->virtual_address;
			size_t section_end = section_va + pe.sections[i]->size_of_raw_data;

			if (section_va <= directory_va) {
				//printf("Considering directory %s in section %s\n", data_directory_names[d], pe.sections[i]->name);
				if (section_end >= directory_va) {
					pe.data_directories[d].section = pe.sections[i];
					pe.data_directories[d].offset = directory_va - section_va;
					pe.data_directories[d].size = directory_size;
					pe.data_directories[d].orig_rva = directory_va;
					pe.data_directories[d].orig_size = directory_size;

					//printf("Found directory %s in section %s\n", data_directory_names[d], pe.sections[i]->name);
				} else {
					//printf("Directory %s doesn't fit in section %s. 0x%08lx < 0x%08lx\n", data_directory_names[d], pe.sections[i]->name, section_end, directory_va);
				}
			}
		}
		print_section(pe.sections[i]);
		printf("\n");
	}

	deserialize_certificate_table(file, &pe.header, size, &pe.certificate_table);
	print_certificate_table(&pe.certificate_table);
	printf("\n");

	pe.start_of_sections = pe.sections[0]->virtual_address;

	//void* t = pe.sections[4];
	//pe.sections[4] = pe.sections[3];
	//pe.sections[3] = t;

	pe.stub = malloc(pe.pe_header_offset);
	if (!pe.stub) {
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

}
#endif
