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

#ifndef PPELIB_H_
#define PPELIB_H_

#include <stddef.h>
#include <inttypes.h>

#include <ppelib/ppelib-constants.h>

#include <ppelib/ppelib-data-directory.h>
#include <ppelib/ppelib-header.h>
#include <ppelib/ppelib-section.h>

typedef struct ppelib_handle_s ppelib_handle;

const char* ppelib_error();

ppelib_handle* ppelib_create();
ppelib_handle* ppelib_create_from_buffer(const uint8_t *buffer, size_t size);
ppelib_handle* ppelib_create_from_file(const char *filename);
void ppelib_destroy(ppelib_handle *pe);

const ppelib_data_directory* ppelib_data_directory_get(ppelib_handle *handle, uint32_t data_directory_index);

const ppelib_section* ppelib_section_get(ppelib_handle* handle, uint16_t section_index);

const ppelib_header* ppelib_header_get(ppelib_handle* handle);
ppelib_header* ppelib_header_copy(ppelib_header* header);
void ppelib_header_free_copy(ppelib_header* header);

uint32_t ppelib_header_compare(ppelib_header* header1, ppelib_header* header2);

#endif /* PPELIB_H_ */
