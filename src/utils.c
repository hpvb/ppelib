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
#include <stdlib.h>
#include <string.h>

#include "platform.h"
#include "utils.h"

uint8_t read_uint8_t(const uint8_t *buffer) {
	return *buffer;
}

void write_uint8_t(uint8_t *buffer, uint8_t val) {
	buffer[0] = val;
}

uint16_t read_uint16_t(const uint8_t *buffer) {
	uint16_t retval;
	memcpy(&retval, buffer, sizeof(uint16_t));
	return retval;
}

void write_uint16_t(uint8_t *buffer, uint16_t val) {
	memcpy(buffer, &val, sizeof(uint16_t));
}

uint32_t read_uint32_t(const uint8_t *buffer) {
	uint32_t retval;
	memcpy(&retval, buffer, sizeof(uint32_t));
	return retval;
}

void write_uint32_t(uint8_t *buffer, uint32_t val) {
	memcpy(buffer, &val, sizeof(uint32_t));
}

uint64_t read_uint64_t(const uint8_t *buffer) {
	uint64_t retval;
	memcpy(&retval, buffer, sizeof(uint64_t));
	return retval;
}

void write_uint64_t(uint8_t *buffer, uint64_t val) {
	memcpy(buffer, &val, sizeof(uint64_t));
}

uint16_t buffer_excise(uint8_t **buffer, size_t size, size_t start, size_t end) {
	if (start >= end) {
		return 0;
	}

	if (size - (end - start) == 0) {
		free(*buffer);
		*buffer = NULL;
	}

	if (end != size) {
		memmove((*buffer) + start, (*buffer) + end, size - end);
	}

	uint8_t *oldptr = *buffer;
	*buffer = realloc(*buffer, size - (end - start));
	if (!*buffer) {
		*buffer = oldptr;
		return 0;
	}

	return 1;
}

uint32_t next_pow2(uint32_t number) {
	number--;
	number |= number >> 1;
	number |= number >> 2;
	number |= number >> 4;
	number |= number >> 8;
	number |= number >> 16;
	number++;

	number = (number == 1) ? 2 : number;
	return number;
}

// TODO find actual hard information on this
uint32_t get_machine_page_size(enum ppelib_machine_type machine) {
	switch (machine) {
	case IMAGE_FILE_MACHINE_IA64:
	case IMAGE_FILE_MACHINE_ALPHA:
	case IMAGE_FILE_MACHINE_ALPHA64:
		return 0x2000;
	default:
		return 0x1000;
	}
}

EXPORT_SYM const char *map_lookup(uint32_t value, const ppelib_map_entry_t *map) {
	const ppelib_map_entry_t *m = map;
	while (m->string) {
		if (m->value == value) {
			return m->string;
		}
		++m;
	}

	return NULL;
}
