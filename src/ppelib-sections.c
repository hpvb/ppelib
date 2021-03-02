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
#include <inttypes.h>
#include <string.h>

#include "ppelib-internal.h"

void ppelib_section_excise(ppelib_file_t *pe, uint16_t section_index, size_t start, size_t end) {
	ppelib_reset_error();

	if (section_index > pe->header.number_of_sections) {
		ppelib_set_error("Section index out of range");
		return;
	}

	ppelib_section_t *section = pe->sections[section_index];
	size_t data_size = MIN(section->virtual_size, section->size_of_raw_data);

	if (end > data_size) {
		ppelib_set_error("Can't delete past section end");
		return;
	}

	if (start >= end) {
		return;
	}

	uint16_t retval = buffer_excise(&pe->sections[section_index]->contents, data_size, start, end);
	if (!retval) {
		ppelib_set_error("Failed to allocate new section contents");
		return;
	}

	if (end - start > UINT32_MAX) {
		ppelib_set_error("Section offset out of range");
		return;
	}

	section->virtual_size -= (uint32_t)(end - start);
	section->size_of_raw_data = TO_NEAREST(section->virtual_size, pe->header.file_alignment);
}

void ppelib_section_resize(ppelib_file_t *pe, uint16_t section_index, size_t size) {
	ppelib_reset_error();

	if (section_index > pe->header.number_of_sections) {
		ppelib_set_error("Section index out of range");
		return;
	}

	ppelib_section_t *section = pe->sections[section_index];
	size_t data_size = MIN(section->virtual_size, section->size_of_raw_data);

	if (size < data_size) {
		ppelib_section_excise(pe, section_index, size, data_size);
		return;
	}

	uint8_t *oldptr = section->contents;
	section->contents = realloc(section->contents, size);
	if (!section->contents) {
		ppelib_set_error("Failed to allocate new section contents");
		section->contents = oldptr;
		return;
	}
}

uint16_t ppelib_section_find_index(ppelib_file_t *pe, ppelib_section_t *section) {
	ppelib_reset_error();

	for (uint16_t i = 0; i < pe->header.number_of_sections; ++i){
		if (pe->sections[i] == section) {
			return i;
		}
	}

	ppelib_set_error("Section not found");
	return 0;
}
