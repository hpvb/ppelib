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

#include <inttypes.h>
#include <stddef.h>

#include <ppelib/ppelib-constants.h>

#include <ppelib/ppelib-data-directory.h>
#include <ppelib/ppelib-dos_header.h>
#include <ppelib/ppelib-header.h>
#include <ppelib/ppelib-section.h>
#include <ppelib/ppelib-vlv_signature.h>

typedef struct ppelib_handle_s ppelib_handle;
typedef struct ppelib_rich_table_s ppelib_rich_table;
typedef struct ppelib_import_table_s ppelib_import_table;

const char *ppelib_error();

ppelib_handle *ppelib_create();
ppelib_handle *ppelib_create_from_buffer(const uint8_t *buffer, size_t size);
ppelib_handle *ppelib_create_from_file(const char *filename);
size_t ppelib_write_to_buffer(ppelib_handle *pe, const uint8_t *buffer, size_t size);
size_t ppelib_write_to_file(ppelib_handle *pe, const char *filename);

void ppelib_destroy(ppelib_handle *pe);

uint8_t *ppelib_get_overlay_data(const ppelib_handle *handle);
size_t ppelib_get_overlay_size(const ppelib_handle *handle);
void ppelib_set_overlay_data(ppelib_handle *handle, const uint8_t *buffer, size_t size);

const ppelib_data_directory *ppelib_data_directory_get(ppelib_handle *handle, uint32_t data_directory_index);

const ppelib_section *ppelib_section_get(ppelib_handle *handle, uint16_t section_index);

// DOS Stub API
ppelib_dos_header *ppelib_dos_header_get(ppelib_handle *handle);
const char *ppelib_dos_header_get_message(const ppelib_dos_header *dos_header);
void ppelib_dos_header_set_message(ppelib_dos_header *dos_header, const char *message);
void ppelib_dos_header_delete_vlv_signature(ppelib_dos_header *dos_header);
void ppelib_dos_header_delete_rich_table(ppelib_dos_header *dos_header);

// DOS Stub VLV API
char ppelib_dos_header_has_vlv_signature(const ppelib_dos_header *dos_header);
const ppelib_vlv_signature *ppelib_dos_header_get_vlv_signature(const ppelib_dos_header *dos_header);
size_t ppelib_vlv_signature_get_signature_size(const ppelib_vlv_signature *vlv_signature);
const uint8_t *ppelib_vlv_signature_get_signature(const ppelib_vlv_signature *vlv_signature);

// DOS Stub Rich table API
char ppelib_dos_header_has_rich_table(const ppelib_dos_header *dos_header);
const ppelib_rich_table *ppelib_dos_header_get_rich_table(const ppelib_dos_header *dos_header);
size_t ppelib_rich_table_get_size(const ppelib_rich_table *table);
uint16_t ppelib_rich_table_get_id(const ppelib_rich_table *table, size_t table_index);
uint16_t ppelib_rich_table_get_build_number(const ppelib_rich_table *table, size_t table_index);
uint32_t ppelib_rich_table_get_use_count(const ppelib_rich_table *table, size_t table_index);
void ppelib_rich_table_fprint(FILE *stream, const ppelib_rich_table *table);
void ppelib_rich_table_print(const ppelib_rich_table *table);

// Header API
ppelib_header *ppelib_header_get(ppelib_handle *handle);
ppelib_header *ppelib_header_copy(ppelib_header *header);
void ppelib_header_free_copy(ppelib_header *header);

uint32_t ppelib_header_compare(ppelib_header *header1, ppelib_header *header2);
uint32_t ppelib_header_compare_non_volitile(ppelib_header *header1, ppelib_header *header2);

// Import table
ppelib_import_table *ppelib_get_import_table(ppelib_handle *handle);
void ppelib_import_table_print(ppelib_import_table *import_table);
#endif /* PPELIB_H_ */
