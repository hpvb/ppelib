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

#include "header_private.h"
#include "section_private.h"
#include "dos_header_private.h"
#include "data_directory_private.h"

typedef struct ppelib_file {
	dos_header_t dos_header;
	size_t header_offset;
	size_t section_offset;
	size_t start_of_section_data;
	size_t end_of_section_data;

	size_t entrypoint_offset;
	section_t *entrypoint_section;

	header_t header;
	data_directory_t *data_directories;
	section_t **sections;

//	certificate_table_t certificate_table;
//	ppelib_resource_table_t resource_table;

	uint8_t *stub;
	size_t trailing_data_size;
	uint8_t *trailing_data;
} ppelib_file_t;

#endif /* PPELIB_MAIN_H_ */
