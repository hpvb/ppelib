/* Copyright 2021 Hein-Pieter van Braam-Stewart
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

#ifndef PELIB_MAIN_H_
#define PELIB_MAIN_H_

#include "pelib-header.h"
#include "pelib-section.h"
#include "pelib-certificate_table.h"
#include "utils.h"

typedef struct data_directory {
	pelib_section_t *section;
	uint32_t offset;
	uint32_t size;

	uint32_t orig_rva;
	uint32_t orig_size;
} data_directory_t;

typedef struct pelib_file {
	size_t pe_header_offset;
	size_t coff_header_offset;
	size_t section_offset;
	size_t start_of_sections;
	size_t end_of_sections;

	pelib_header_t header;
	pelib_section_t **sections;
	data_directory_t *data_directories;

	pelib_certificate_table_t certificate_table;

	uint8_t *stub;
	size_t trailing_data_size;
	uint8_t *trailing_data;

	uint8_t *file_contents;
	size_t file_size;
} pelib_file_t;

#endif /* PELIB_MAIN_H_ */
