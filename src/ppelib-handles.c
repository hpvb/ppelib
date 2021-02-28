/* Copyright 2021 Hein-Pieter van Braam-Stewart
 *
 * This file is part of ppelib (Portable Portable Executable LIBrary)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <ppelib/ppelib-constants.h>
#include <ppelib-error.h>
#include <ppelib-internal.h>
#include "export.h"
#include "main.h"

EXPORT_SYM ppelib_file_t* ppelib_create() {
	ppelib_reset_error();

	ppelib_file_t *pe = calloc(sizeof(ppelib_file_t), 1);
	if (!pe) {
		ppelib_set_error("Failed to allocate PE structure");
	}

	return pe;
}

EXPORT_SYM void ppelib_destroy(ppelib_file_t *pe) {
	if (ppelib_error())
		printf("Last error: %s\n", ppelib_error());

	if (!pe) {
		return;
	}

	ppelib_free_certificate_table(&pe->certificate_table);
	free_resource_directory(pe);

	free(pe->stub);
	if (pe->allocated_sections) {
		for (size_t i = 0; i < pe->header.number_of_sections; ++i) {
			free(pe->sections[i]->contents);
			free(pe->sections[i]);
		}
	}
	free(pe->data_directories);
	free(pe->header.data_directories);
	free(pe->sections);
	free(pe->trailing_data);

	free(pe);
}

EXPORT_SYM ppelib_file_t* ppelib_create_from_buffer(const uint8_t *buffer, size_t size) {
	ppelib_reset_error();

	if (size < PE_SIGNATURE_OFFSET + sizeof(uint32_t)) {
		ppelib_set_error("Not a PE file (file too small)");
		return NULL;
	}

	uint32_t header_offset = read_uint32_t(buffer + PE_SIGNATURE_OFFSET);
	if (size < header_offset + sizeof(uint32_t)) {
		ppelib_set_error("Not a PE file (file too small for PE signature)");
		return NULL;
	}

	uint32_t signature = read_uint32_t(buffer + header_offset);
	if (signature != PE_SIGNATURE) {
		ppelib_set_error("Not a PE file (PE00 signature missing)");
		return NULL;
	}

	ppelib_file_t *pe = ppelib_create();
	if (ppelib_error_peek()) {
		ppelib_destroy(pe);
		return NULL;
	}

	pe->pe_header_offset = header_offset;
	pe->coff_header_offset = header_offset + 4;

	if (size < pe->coff_header_offset + COFF_HEADER_SIZE) {
		ppelib_set_error("Not a PE file (file too small for COFF header)");
		ppelib_destroy(pe);
		return NULL;
	}

	size_t header_size = deserialize_pe_header(buffer, pe->coff_header_offset, size, &pe->header);
	if (ppelib_error_peek()) {
		ppelib_destroy(pe);
		return NULL;
	}

	pe->section_offset = header_size + pe->coff_header_offset;
	pe->sections = malloc(sizeof(ppelib_section_t*) * pe->header.number_of_sections);
	if (!pe->sections) {
		ppelib_set_error("Failed to allocate sections");
		ppelib_destroy(pe);
		return NULL;
	}

	pe->data_directories = calloc(sizeof(ppelib_data_directory_t) * pe->header.number_of_rva_and_sizes, 1);
	if (!pe->data_directories) {
		ppelib_set_error("Failed to allocate data directories");
		ppelib_destroy(pe);
		return NULL;
	}

	pe->end_of_sections = 0;

	for (uint32_t i = 0; i < pe->header.number_of_sections; ++i) {
		pe->sections[i] = calloc(sizeof(ppelib_section_t), 1);
		if (!pe->sections[i]) {
			pe->header.number_of_sections = i;
			ppelib_set_error("Failed to allocate section");
			ppelib_destroy(pe);
			return NULL;
		}
	}
	pe->allocated_sections = 1;

	for (uint32_t i = 0; i < pe->header.number_of_sections; ++i) {
		size_t section_size = deserialize_section(buffer, pe->section_offset + (i * PE_SECTION_HEADER_SIZE), size,
				pe->sections[i]);

		if (ppelib_error_peek()) {
			ppelib_destroy(pe);
			return NULL;
		}

		if (pe->sections[i]->pointer_to_raw_data > size) {
			ppelib_set_error("Section past end of file");
			ppelib_destroy(pe);
			return NULL;
		}

		if (section_size > pe->end_of_sections) {
			pe->end_of_sections = section_size;
		}

		for (uint32_t d = 0; d < pe->header.number_of_rva_and_sizes; ++d) {
			size_t directory_va = pe->header.data_directories[d].virtual_address;
			size_t directory_size = pe->header.data_directories[d].size;
			size_t section_va = pe->sections[i]->virtual_address;
			size_t section_va_end = section_va + pe->sections[i]->size_of_raw_data;

			if (d != DIR_CERTIFICATE_TABLE) {
				if (section_va <= directory_va && section_va_end >= directory_va) {
					pe->data_directories[d].section = pe->sections[i];
					pe->data_directories[d].offset = directory_va - section_va;
					pe->data_directories[d].size = directory_size;
					pe->data_directories[d].orig_rva = directory_va;
					pe->data_directories[d].orig_size = directory_size;
				}
			}
		}
	}

	if (pe->header.number_of_rva_and_sizes >= DIR_CERTIFICATE_TABLE) {
		if (pe->header.data_directories[DIR_CERTIFICATE_TABLE].size) {
			deserialize_certificate_table(buffer, &pe->header, size, &pe->certificate_table);
			if (ppelib_error_peek()) {
				ppelib_destroy(pe);
				return NULL;
			}
		}
	}

	if (pe->header.number_of_sections) {
		pe->start_of_sections = pe->sections[0]->virtual_address;
	}

	//void* t = pe.sections[4];
	//pe.sections[4] = pe.sections[3];
	//pe.sections[3] = t;

	pe->stub = malloc(pe->pe_header_offset);
	if (!pe->stub) {
		ppelib_set_error("Failed to allocate memory for PE stub");
		ppelib_destroy(pe);
		return NULL;
	}
	memcpy(pe->stub, buffer, pe->pe_header_offset);

	if (size > pe->end_of_sections) {
		pe->trailing_data_size = size - pe->end_of_sections;
		pe->trailing_data = malloc(pe->trailing_data_size);

		if (!pe->trailing_data) {
			ppelib_set_error("Failed to allocate memory for trailing data");
			ppelib_destroy(pe);
			return NULL;
		}

		memcpy(pe->trailing_data, buffer + pe->end_of_sections, pe->trailing_data_size);
	}

	if (pe->header.number_of_rva_and_sizes >= DIR_RESOURCE_TABLE) {
		if (pe->header.data_directories[DIR_RESOURCE_TABLE].size) {
			parse_resource_table(pe);
		}
	}

	return pe;
}

EXPORT_SYM ppelib_file_t* ppelib_create_from_file(const char *filename) {
	ppelib_reset_error();
	size_t file_size;
	uint8_t *file_contents;

	FILE *f = fopen(filename, "rb");

	if (!f) {
		ppelib_set_error("Failed to open file");
		return NULL;
	}

	fseek(f, 0, SEEK_END);
	file_size = ftell(f);
	rewind(f);

	file_contents = malloc(file_size);
	if (!file_size) {
		fclose(f);
		ppelib_set_error("Failed to allocate file data");
		return NULL;
	}

	size_t retsize = fread(file_contents, 1, file_size, f);
	if (retsize != file_size) {
		fclose(f);
		ppelib_set_error("Failed to read file data");
		return NULL;
	}

	fclose(f);

	ppelib_file_t *retval = ppelib_create_from_buffer(file_contents, file_size);
	free(file_contents);

	return retval;
}

EXPORT_SYM size_t ppelib_write_to_buffer(ppelib_file_t *pe, uint8_t *buffer, size_t buf_size) {
	ppelib_reset_error();

	size_t size = 0;

	// Write stub
	size += pe->pe_header_offset;
	size += 4;
	size_t coff_header_size = serialize_pe_header(&pe->header, NULL, pe->pe_header_offset);
	if (ppelib_error_peek()) {
		return 0;
	}

	size += coff_header_size;
	size_t end_of_sections = 0;

	size_t section_offset = pe->pe_header_offset + coff_header_size;
	for (uint32_t i = 0; i < pe->header.number_of_sections; ++i) {
		size_t section_size = serialize_section(pe->sections[i], NULL, section_offset + (i * PE_SECTION_HEADER_SIZE));
		if (ppelib_error_peek()) {
			return 0;
		}

		if (section_size > end_of_sections) {
			end_of_sections = section_size;
		}
	}

	// Theoretically all the sections could be before the header
	if (end_of_sections > size) {
		size = end_of_sections;
	}

	size += pe->trailing_data_size;

	size_t certificates_size = 0;
	if (pe->header.data_directories[DIR_CERTIFICATE_TABLE].size) {
		certificates_size = serialize_certificate_table(&pe->certificate_table, NULL);
		if (ppelib_error_peek()) {
			return 0;
		}

		if (certificates_size > size) {
			size = certificates_size;
		}
	}

	if (buffer && size > buf_size) {
		ppelib_set_error("Target buffer too small.");
		return 0;
	}

	if (!buffer) {
		return size;
	}

	size_t write = 0;

	memset(buffer, 0, size);
	//memset(buffer, 0xCC, size);

	memcpy(buffer, pe->stub, pe->pe_header_offset);
	write += pe->pe_header_offset;

	// Write PE header
	memcpy(buffer + write, "PE\0", 4);
	write += 4;

	// Write COFF header
	serialize_pe_header(&pe->header, buffer, pe->pe_header_offset + 4);
	if (ppelib_error_peek()) {
		return 0;
	}

	// Write sections
	for (uint32_t i = 0; i < pe->header.number_of_sections; ++i) {
		serialize_section(pe->sections[i], buffer, section_offset + 4 + (i * PE_SECTION_HEADER_SIZE));
		if (ppelib_error_peek()) {
			return 0;
		}
	}

	// Write trailing data
	memcpy(buffer + end_of_sections, pe->trailing_data, pe->trailing_data_size);

	// Write certificates
	serialize_certificate_table(&pe->certificate_table, buffer);
	if (ppelib_error_peek()) {
		return 0;
	}

	return size;
}

EXPORT_SYM size_t ppelib_write_to_file(ppelib_file_t *pe, const char *filename) {
	ppelib_reset_error();

	FILE *f = fopen(filename, "wb");
	if (!f) {
		ppelib_set_error("Failed to open file");
		return 0;
	}

	size_t bufsize = ppelib_write_to_buffer(pe, NULL, 0);
	if (ppelib_error_peek()) {
		fclose(f);
		return 0;
	}

	uint8_t *buffer = malloc(bufsize);
	if (!buffer) {
		ppelib_set_error("Failed to allocate buffer");
		fclose(f);
		return 0;
	}

	ppelib_write_to_buffer(pe, buffer, bufsize);
	if (ppelib_error_peek()) {
		fclose(f);
		free(buffer);
		return 0;
	}

	size_t written = fwrite(buffer, 1, bufsize, f);
	fclose(f);
	free(buffer);

	if (written != bufsize) {
		ppelib_set_error("Failed to write data");
	}

	return written;
}

EXPORT_SYM void ppelib_recalculate(ppelib_file_t *pe) {
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
		ppelib_section_t *section = pe->sections[i];

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

	// PE files with only data can have this set to garbage. Might as well just keep it.
	if (size_of_code) {
		pe->header.base_of_code = base_of_code;
	}

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

EXPORT_SYM int LLVMFuzzerTestOneInput(const uint8_t *buffer, size_t size) {
	ppelib_file_t *pe = ppelib_create_from_buffer(buffer, size);
	if (ppelib_error()) {
		printf("PPELib-Error: %s\n", ppelib_error());
	} else {
		ppelib_print_pe_header(&pe->header);
		ppelib_print_resource_table(&pe->resource_table);
		size_t len = ppelib_write_to_buffer(pe, NULL, 0);
		uint8_t *buffer = malloc(len);
		ppelib_write_to_buffer(pe, buffer, len);
		free(buffer);
	}
	ppelib_destroy(pe);
	return 0;  // Non-zero return values are reserved for future use.
}

