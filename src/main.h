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

#ifndef PPELIB_MAIN_H_
#define PPELIB_MAIN_H_

#include "ppelib-header.h"
#include "ppelib-section.h"
#include "ppelib-certificate_table.h"
#include "utils.h"

typedef struct ppelib_data_directory {
	ppelib_section_t *section;
	uint32_t offset;
	uint32_t size;

	uint32_t orig_rva;
	uint32_t orig_size;
} ppelib_data_directory_t;

typedef struct ppelib_file {
	size_t pe_header_offset;
	size_t coff_header_offset;
	size_t section_offset;
	size_t start_of_sections;
	size_t end_of_sections;

	ppelib_header_t header;
	ppelib_section_t **sections;
	ppelib_data_directory_t *data_directories;

	ppelib_certificate_table_t certificate_table;

	uint8_t *stub;
	size_t trailing_data_size;
	uint8_t *trailing_data;
} ppelib_file_t;

#endif /* PPELIB_MAIN_H_ */
