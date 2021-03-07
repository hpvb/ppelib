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

#include <inttypes.h>
#include <ppe_error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ppelib/ppelib-constants.h>

#include "ppelib_internal.h"
#include "main.h"

EXPORT_SYM uint8_t* ppelib_get_trailing_data(const ppelib_file_t *pe) {
	ppelib_reset_error();

	return pe->trailing_data;
}

EXPORT_SYM size_t ppelib_get_trailing_data_size(const ppelib_file_t *pe) {
	ppelib_reset_error();

	return pe->trailing_data_size;
}

EXPORT_SYM void ppelib_set_trailing_data(ppelib_file_t *pe, const uint8_t *buffer, size_t size) {
	ppelib_reset_error();

	if (!buffer && size) {
		ppelib_set_error("Can't set data from a NULL pointer");
		return;
	}

	void *oldptr = pe->trailing_data;

	if (!buffer || !size) {
		pe->trailing_data = NULL;
		pe->trailing_data_size = 0;
	} else {
		pe->trailing_data = malloc(size);
		if (!pe->trailing_data) {
			ppelib_set_error("Failed to allocate new trailing data");
			pe->trailing_data = oldptr;
			return;
		}

		memcpy(pe->trailing_data, buffer, size);
		pe->trailing_data_size = 0;
	}

	free(oldptr);
}

EXPORT_SYM ppelib_file_t* ppelib_create() {
	ppelib_reset_error();

	ppelib_file_t *pe = calloc(sizeof(ppelib_file_t), 1);
	if (!pe) {
		ppelib_set_error("Failed to allocate PE structure");
	}

	return pe;
}

EXPORT_SYM void ppelib_destroy(ppelib_file_t *pe) {
	if (!pe) {
		return;
	}

	if (pe->sections) {
		for (uint16_t i = 0; i < pe->header.number_of_sections; ++i) {
			free(pe->sections[i]->contents);
			free(pe->sections[i]);
		}
	}

	free(pe->dos_header.stub);
	free(pe->dos_header.message);
	free(pe->dos_header.vlv_signature.signature);
	free(pe->dos_header.rich_table.entries);
	free(pe->data_directories);
	free(pe->sections);
	free(pe->trailing_data);

	free(pe);
}

EXPORT_SYM ppelib_file_t* ppelib_create_from_buffer(const uint8_t *buffer, size_t size) {
	ppelib_reset_error();

	if (size < 2) {
		ppelib_set_error("Not a PE file (too small for MZ signature)");
		return NULL;
	}

	uint16_t mz_signature = read_uint16_t(buffer);
	if (mz_signature != MZ_SIGNATURE) {
		ppelib_set_error("Not a PE file (MZ signature missing)");
		return NULL;
	}

	ppelib_file_t *pe = ppelib_create();
	if (ppelib_error_peek()) {
		ppelib_destroy(pe);
		return NULL;
	}

	size_t dos_header_size = ppelib_dos_header_deserialize(buffer, size, 2, &pe->dos_header);
	if (ppelib_error_peek()) {
		ppelib_destroy(pe);
		return NULL;
	}

	pe->dos_header.pe = pe;

	if (size < pe->dos_header.pe_header_offset + sizeof(uint32_t)) {
		ppelib_set_error("Not a PE file (file too small)");
		ppelib_destroy(pe);
		return NULL;
	}

	if (pe->dos_header.pe_header_offset >= dos_header_size) {
		size_t dos_stub_size = pe->dos_header.pe_header_offset - dos_header_size;
		pe->dos_header.stub = malloc(dos_stub_size);
		if (!pe->dos_header.stub) {
			ppelib_set_error("Couldn't allocate DOS stub");
			ppelib_destroy(pe);
			return NULL;
		}

		memcpy(pe->dos_header.stub, buffer + 2 + dos_header_size, dos_stub_size);
		pe->dos_header.stub_size = dos_stub_size;
		parse_dos_stub(&pe->dos_header);
	}

	uint32_t signature = read_uint32_t(buffer + pe->dos_header.pe_header_offset);
	if (signature != PE_SIGNATURE) {
		ppelib_set_error("Not a PE file (PE00 signature missing)");
		ppelib_destroy(pe);
		return NULL;
	}

	size_t header_offset = pe->dos_header.pe_header_offset + 4;

	size_t header_size = ppelib_header_deserialize(buffer, size, header_offset, &pe->header);
	if (ppelib_error_peek()) {
		ppelib_destroy(pe);
		return NULL;
	}

	pe->header.pe = pe;

	if (pe->header.number_of_rva_and_sizes > (UINT32_MAX / DATA_DIRECTORY_SIZE)) {
		ppelib_set_error("File too small for directory entries (overflow)");
		ppelib_destroy(pe);
		return NULL;
	}

	size_t data_directories_size = (pe->header.number_of_rva_and_sizes * DATA_DIRECTORY_SIZE);
	if (header_offset + header_size + data_directories_size > size) {
		ppelib_set_error("File too small for directory entries");
		ppelib_destroy(pe);
		return NULL;
	}

	size_t section_offset = header_offset + COFF_HEADER_SIZE + pe->header.size_of_optional_header;
	pe->start_of_section_data = ((size_t) (pe->header.number_of_sections) * SECTION_SIZE) + section_offset;
	if (pe->start_of_section_data > size) {
		ppelib_set_error("File too small for section headers");
		ppelib_destroy(pe);
		return NULL;
	}

	pe->sections = calloc(sizeof(void*) * pe->header.number_of_sections, 1);
	if (!pe->sections) {
		ppelib_set_error("Failed to allocate sections array");
		ppelib_destroy(pe);
		return NULL;
	}

	for (uint16_t i = 0; i < pe->header.number_of_sections; ++i) {
		pe->sections[i] = calloc(sizeof(section_t), 1);
		if (!pe->sections[i]) {
			for (uint16_t s = 0; s < i; ++s) {
				free(pe->sections[s]);
			}
			memset(pe->sections, 0, sizeof(void*) * pe->header.number_of_sections);
			ppelib_set_error("Failed to allocate sections");
			ppelib_destroy(pe);
			return NULL;
		}
	}

	size_t offset = section_offset;
	pe->start_of_section_va = 0;
	pe->end_of_section_data = pe->start_of_section_data;
	char first_section = 1;

	for (uint16_t i = 0; i < pe->header.number_of_sections; ++i) {
		size_t section_size = ppelib_section_deserialize(buffer, size, offset, pe->sections[i]);
		if (ppelib_error_peek()) {
			ppelib_destroy(pe);
			return NULL;
		}

		section_t *section = pe->sections[i];
		section->pe = pe;

		if (i == 0) {
			pe->start_of_section_va = section->virtual_address;
		} else {
			pe->start_of_section_va = MIN(pe->start_of_section_va, section->virtual_address);
		}

		if (section->size_of_raw_data > section->size_of_raw_data + section->virtual_size) {
			ppelib_set_error("Section data size out of range");
			ppelib_destroy(pe);
			return NULL;
		}

		size_t data_size = MIN(section->virtual_size, section->size_of_raw_data);

		if (section->pointer_to_raw_data + data_size > size || section->pointer_to_raw_data > size
				|| data_size > size) {
			ppelib_set_error("Section data outside of file");
			ppelib_destroy(pe);
			return NULL;
		}

		section->contents = malloc(data_size);
		if (!section->contents) {
			ppelib_set_error("Failed to allocate section data");
			ppelib_destroy(pe);
			return NULL;
		}

		section->contents_size = data_size;
		memcpy(section->contents, buffer + section->pointer_to_raw_data, section->contents_size);

		if (section->pointer_to_raw_data) {
			if (first_section) {
				first_section = 0;
				pe->start_of_section_data = section->pointer_to_raw_data;
			} else {
				pe->start_of_section_data = MIN(pe->start_of_section_data, section->pointer_to_raw_data);
			}
		}

		pe->end_of_section_data = MAX(pe->end_of_section_data,
				section->pointer_to_raw_data + section->size_of_raw_data);
		offset += section_size;
	}

	pe->data_directories = calloc(sizeof(data_directory_t) * pe->header.number_of_rva_and_sizes, 1);
	if (!pe->data_directories) {
		ppelib_set_error("Failed to allocate data directories");
		ppelib_destroy(pe);
		return NULL;
	}

	// Data directories don't have a dedicated deserialize function
	offset = header_offset + header_size;
	for (uint32_t i = 0; i < pe->header.number_of_rva_and_sizes; ++i) {
		uint32_t dir_va = read_uint32_t(buffer + offset + 0);
		uint32_t dir_size = read_uint32_t(buffer + offset + 4);

		section_t *section = section_find_by_virtual_address(pe, dir_va);
		if (i != DIR_CERTIFICATE_TABLE && section) {
			pe->data_directories[i].section = section;
			pe->data_directories[i].offset = dir_va - section->virtual_address;
		} else if (dir_size) {
			// Certificate tables' addresses aren't virtual. Despite the name.
			pe->data_directories[i].offset = dir_va - pe->end_of_section_data;
		}
		pe->data_directories[i].size = dir_size;
		pe->data_directories[i].id = i;

		offset += DATA_DIRECTORY_SIZE;
	}

	pe->end_of_section_data = MAX(pe->end_of_section_data, header_offset + header_size);
	if (size > pe->end_of_section_data) {
		pe->trailing_data_size = size - pe->end_of_section_data;
		pe->trailing_data = malloc(pe->trailing_data_size);
		if (!pe->trailing_data) {
			ppelib_set_error("Failed to allocate trailing data");
			ppelib_destroy(pe);
			return NULL;
		}

		memcpy(pe->trailing_data, buffer + pe->end_of_section_data, pe->trailing_data_size);
	}

	pe->entrypoint_section = section_find_by_virtual_address(pe, pe->header.address_of_entry_point);

	if (pe->entrypoint_section) {
		pe->entrypoint_offset = pe->header.address_of_entry_point - pe->entrypoint_section->virtual_address;
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
	long ftell_size = ftell(f);
	rewind(f);

	if (ftell_size < 0) {
		fclose(f);
		ppelib_set_error("Unable to read file length");
		return NULL;
	}

	file_size = (size_t) ftell_size;

	if (!file_size) {
		fclose(f);
		ppelib_set_error("Empty file");
		return NULL;
	}

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
	size_t size = 0;

//	size_t dos_stub_size = pe->dos_header.stub_size;
	size_t header_size = ppelib_header_serialize(&pe->header, NULL, 0);
	size_t data_tables_size = pe->header.number_of_rva_and_sizes * DATA_DIRECTORY_SIZE;
	size_t section_header_size = pe->header.number_of_sections * SECTION_SIZE;

	size_t section_size = 0;

	size_t pe_header_offset = pe->dos_header.pe_header_offset + 4;
	size_t section_header_offset = pe_header_offset + COFF_HEADER_SIZE + pe->header.size_of_optional_header;

	uint32_t file_alignment = MAX(pe->header.file_alignment, 512);
	file_alignment = MIN(file_alignment, UINT16_MAX);

	for (uint16_t i = 0; i < pe->header.number_of_sections; ++i) {
		section_t *section = pe->sections[i];

		size_t this_section_size = section->pointer_to_raw_data;
		this_section_size += section->size_of_raw_data;
		section_size = MAX(section_size, this_section_size);
	}

	size += 2;
	size += pe->dos_header.pe_header_offset;
	size += 4;
	size += pe->header.size_of_optional_header;
	size += section_header_size;

	// Some of this stuff may overlap so we need to ensure we have at least as much space
	// as the furthest out write
	size = MAX(size, section_size);
	size = MAX(size, pe_header_offset + header_size);
	size = MAX(size, pe_header_offset + header_size + data_tables_size);
	size = MAX(size, section_header_offset + section_header_size);

	size_t end_of_section_data = size;

	size += pe->trailing_data_size;

	printf("dos_header_size: %i\n", DOS_HEADER_SIZE);
	//printf("dos_stub_size: %zi\n", dos_stub_size);
	printf("header_size: %zi\n", header_size);
	//printf("data_tables_size: %zi\n", data_tables_size);
	printf("section_header_size: %zi\n", section_header_size);
	printf("section_size: %zi\n", section_size);
	printf("trailing_size: %zi\n", pe->trailing_data_size);
	printf("total: %zi\n", size);

	if (!buffer) {
		return size;
	}

	if (buffer && size > buf_size) {
		ppelib_set_error("Target buffer too small.");
		return 0;
	}

	memset(buffer, 0, size);

	write_uint16_t(buffer, MZ_SIGNATURE);
	ppelib_dos_header_serialize(&pe->dos_header, buffer, 2);
	if (pe->dos_header.stub_size) {
		memcpy(buffer + 2 + DOS_HEADER_SIZE, pe->dos_header.stub, pe->dos_header.stub_size);
	}
	write_uint32_t(buffer + pe->dos_header.pe_header_offset, PE_SIGNATURE);
	ppelib_header_serialize(&pe->header, buffer, pe_header_offset);

	size_t offset = pe_header_offset + header_size;
	for (uint32_t i = 0; i < pe->header.number_of_rva_and_sizes; ++i) {
		data_directory_t *dir = &pe->data_directories[i];
		section_t *section = dir->section;
		uint32_t dir_va = 0;
		uint32_t dir_size = (uint32_t) dir->size;

		if (section) {
			dir_va = (uint32_t) (section->virtual_address + dir->offset);
		} else if (dir->size) {
			dir_va = (uint32_t) (end_of_section_data + dir->offset);
		}

		write_uint32_t(buffer + offset + 0, dir_va);
		write_uint32_t(buffer + offset + 4, dir_size);

		offset += DATA_DIRECTORY_SIZE;
	}

	offset = section_header_offset;
	for (uint16_t i = 0; i < pe->header.number_of_sections; ++i) {
		section_t *section = pe->sections[i];
		ppelib_section_serialize(section, buffer, offset);

		if (section->contents_size) {
			memcpy(buffer + section->pointer_to_raw_data, section->contents, section->contents_size);
		}

		offset += SECTION_SIZE;
	}

	if (pe->trailing_data_size) {
		memcpy(buffer + end_of_section_data, pe->trailing_data, pe->trailing_data_size);
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

void recalculate_sections(ppelib_file_t *pe) {
	uint32_t base_of_code = 0;
	uint32_t base_of_data = 0;
	uint32_t size_of_initialized_data = 0;
	uint32_t size_of_uninitialized_data = 0;
	uint32_t size_of_code = 0;

	uint32_t next_section_virtual = (uint32_t) pe->start_of_section_va;
	uint32_t next_section_physical = (uint32_t) pe->start_of_section_data;

	uint32_t machine_page_size = get_machine_page_size(pe->header.machine);
	next_section_virtual = MAX(next_section_virtual, next_section_physical);

	char modified = 0;

	for (uint16_t i = 0; i < pe->header.number_of_sections; ++i) {
		section_t *section = pe->sections[i];

		if (section->modified) {
			modified = 1;
			//section->virtual_size = (uint32_t) (section->contents_size + section->virtual_padding);
			section->size_of_raw_data = TO_NEAREST((uint32_t )section->contents_size, pe->header.file_alignment);
		}

		// SizeOfRawData can't be more than the aligned amount of the data we actually have
		if (section->size_of_raw_data > TO_NEAREST(section->contents_size, pe->header.file_alignment)) {
			modified = 1;
			section->size_of_raw_data = TO_NEAREST((uint32_t ) section->contents_size, pe->header.file_alignment);
		}

		if (section->size_of_raw_data) {
			if (section->pointer_to_raw_data != next_section_physical) {
				uint32_t next_section_physical_aligned = TO_NEAREST(next_section_physical,
						pe->header.section_alignment);

				if (section->pointer_to_raw_data != next_section_physical_aligned) {
					section->pointer_to_raw_data = next_section_physical_aligned;
					modified = 1;
				}
			}
		}

		if (section->virtual_size) {
			if (section->virtual_address != next_section_virtual) {
				section->virtual_address = next_section_virtual;
				modified = 1;
			}

			if (machine_page_size > pe->header.section_alignment) {
				if (section->virtual_address != section->pointer_to_raw_data) {
					section->virtual_address = section->pointer_to_raw_data;
					modified = 1;
				}
			}
		}

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

		if (section->size_of_raw_data) {
			next_section_physical = TO_NEAREST(section->pointer_to_raw_data, pe->header.file_alignment);
			next_section_physical += TO_NEAREST(section->size_of_raw_data, pe->header.file_alignment);
		}

		if (section->virtual_size) {
			next_section_virtual = TO_NEAREST(section->virtual_address, pe->header.section_alignment);
			next_section_virtual += TO_NEAREST(section->virtual_size, pe->header.section_alignment);
		}

		pe->end_of_section_data = MAX(pe->end_of_section_data,
				section->pointer_to_raw_data + section->size_of_raw_data);

		section->modified = 0;
	}

	if (modified) {
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
		pe->header.size_of_image = next_section_virtual;
		if (pe->entrypoint_section) {
			pe->header.address_of_entry_point = pe->entrypoint_section->virtual_address
					+ (uint32_t) pe->entrypoint_offset;
		}
	}
}

void recalculate_header(ppelib_file_t *pe) {
	uint16_t header_size = (uint16_t) ppelib_header_serialize(&pe->header, NULL, 0);
	size_t data_tables_size = pe->header.number_of_rva_and_sizes * DATA_DIRECTORY_SIZE;
	size_t section_header_size = pe->header.number_of_sections * SECTION_SIZE;

	size_t total_header_size = pe->dos_header.pe_header_offset + 4 + header_size + data_tables_size
			+ section_header_size;

	if (!pe->header.file_alignment || pe->header.file_alignment > UINT16_MAX) {
		pe->header.file_alignment = 512;
	}

	if (pe->header.file_alignment > 512) {
		pe->header.file_alignment = next_pow2(pe->header.file_alignment);
	}

	if (!pe->header.section_alignment || pe->header.section_alignment > UINT16_MAX
			|| pe->header.section_alignment < pe->header.file_alignment) {

		pe->header.section_alignment = get_machine_page_size(pe->header.machine);
	}

	if (pe->header.section_alignment > get_machine_page_size(pe->header.machine)) {
		pe->header.section_alignment = next_pow2(pe->header.section_alignment);
	}

	if (TO_NEAREST(total_header_size, pe->header.file_alignment) > UINT32_MAX) {
		pe->header.size_of_headers = 0;
	} else {
		pe->header.size_of_headers = (uint32_t) (TO_NEAREST(total_header_size, pe->header.file_alignment));
	}

	uint16_t old_size_of_optional_header = pe->header.size_of_optional_header;
	pe->header.size_of_optional_header = (uint16_t) data_tables_size;

	if (pe->header.magic == PE32_MAGIC) {
		pe->header.size_of_optional_header += PE_OPTIONAL_HEADER_SIZE;
	}

	if (pe->header.magic == PE32PLUS_MAGIC) {
		pe->header.size_of_optional_header += PEPLUS_OPTIONAL_HEADER_SIZE;
	}

	size_t old_start_of_section_data = pe->start_of_section_data;
	pe->start_of_section_data = MAX(pe->start_of_section_data, pe->header.size_of_headers);

	if (old_size_of_optional_header != pe->header.size_of_optional_header
			|| old_start_of_section_data != pe->start_of_section_data) {
		recalculate_sections(pe);
	}

	pe->header.modified = 0;
}

void recalculate_dos_header(ppelib_file_t *pe) {
	// If anything changed in the DOS area we must update all PE headers
	// since we can't preserve overlapping data here.

	if (pe->dos_header.modified) {
		recalculate_header(pe);
	}

	pe->dos_header.modified = 0;
}

EXPORT_SYM void ppelib_recalculate_force(ppelib_file_t *pe) {
	if (!pe) {
		return;
	}

	recalculate_dos_header(pe);
	recalculate_header(pe);
	recalculate_sections(pe);
}

EXPORT_SYM void ppelib_recalculate(ppelib_file_t *pe) {
	if (!pe) {
		return;
	}

	if (pe->dos_header.modified) {
		recalculate_dos_header(pe);
	}

	if (pe->header.modified) {
		recalculate_header(pe);
	}

	// Doesn't do anything unless there's changes
	recalculate_sections(pe);
}
