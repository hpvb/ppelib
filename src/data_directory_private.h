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

#ifndef PPELIB_DATA_DIRECTORY_PRIVATE_H_
#define PPELIB_DATA_DIRECTORY_PRIVATE_H_

#include <inttypes.h>
#include <stdio.h>
#include <stddef.h>

#include <ppelib/ppelib-constants.h>

#include "main.h"
#include "platform.h"
#include "utils.h"

typedef struct ppelib_file ppelib_file_t;

typedef struct data_directory {
	section_t *section;

	size_t offset;
	size_t size;
	uint32_t id;
} data_directory_t;

EXPORT_SYM uint32_t ppelib_data_directory_get_id(const data_directory_t *data_directory);
EXPORT_SYM const section_t* ppelib_data_directory_get_section(const data_directory_t *data_directory);
EXPORT_SYM size_t ppelib_data_directory_get_offset(const data_directory_t *data_directory);
EXPORT_SYM size_t ppelib_data_directory_get_size(const data_directory_t *data_directory);
EXPORT_SYM uint32_t ppelib_data_directory_get_rva(const data_directory_t *data_directory);

EXPORT_SYM size_t ppelib_data_directory_serialize(const data_directory_t *data_directory, uint8_t *buffer,
		const size_t offset);

EXPORT_SYM const data_directory_t* ppelib_data_directory_get(ppelib_file_t *pe, uint32_t data_directory_index);
EXPORT_SYM void ppelib_data_directory_printf(FILE *stream, const data_directory_t *data_directory);
EXPORT_SYM void ppelib_data_directory_print(const data_directory_t *data_directory);

#endif /* PPELIB_DATA_DIRECTORY_PRIVATE_H_ */
