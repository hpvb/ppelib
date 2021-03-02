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

#ifndef PPELIB_RESOURCE_TABLE_H_
#define PPELIB_RESOURCE_TABLE_H_

#include <stddef.h>
#include <stdint.h>

#include <ppelib/ppelib-constants.h>

typedef struct ppelib_resource_data {
	wchar_t* name;
	uint32_t resource_type;

	uint32_t size;
	uint32_t codepage;
	uint32_t reserved;

	uint8_t* data;
} ppelib_resource_data_t;

typedef struct ppelib_resource_table {
	uint8_t root;

	wchar_t* name;
	uint32_t resource_type;

	uint32_t characteristics;
	uint32_t time_date_stamp;
	uint16_t major_version;
	uint16_t minor_version;

	size_t subdirectories_number;
	struct ppelib_resource_table **subdirectories;

	size_t data_entries_number;
	struct ppelib_resource_data **data_entries;
} ppelib_resource_table_t;

#endif /* PPELIB_RESOURCE_TABLE_H_ */
