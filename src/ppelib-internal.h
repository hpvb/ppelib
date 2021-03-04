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

#ifndef PPELIB_INTERNAL_H_
#define PPELIB_INTERNAL_H_

#include <inttypes.h>
#include <stddef.h>

#include "export.h"
#include "main.h"
#include "ppelib-error.h"
#include "thread_local.h"
#include "utils.h"

size_t deserialize_certificate_table(const uint8_t *buffer, ppelib_file_t *pe, const size_t size, ppelib_certificate_table_t *certificate_table);
size_t deserialize_pe_header(const uint8_t *buffer, size_t offset, const size_t size, ppelib_header_t *header);
size_t deserialize_section(const uint8_t *buffer, size_t offset, const size_t size, ppelib_section_t *section);
size_t parse_resource_table(ppelib_file_t *pe);
size_t serialize_certificate_table(const ppelib_certificate_table_t *certificate_table, uint8_t *buffer, size_t offset);
size_t serialize_pe_header(const ppelib_header_t *header, uint8_t *buffer, size_t offset);
size_t serialize_resource_table(const ppelib_resource_table_t *resource_table, uint8_t *buffer, size_t rscs_base);
size_t serialize_section(const ppelib_section_t *section, uint8_t *buffer, size_t offset);
uint16_t ppelib_section_create(ppelib_file_t *pe, char name[9], uint32_t virtual_size, uint32_t raw_size, uint32_t characteristics, uint8_t *data);
uint16_t ppelib_section_find_index(ppelib_file_t *pe, ppelib_section_t *section);
void free_resource_directory(ppelib_file_t *pe);
void ppelib_free_certificate_table(ppelib_certificate_table_t *certificate_table);
void ppelib_section_excise(ppelib_file_t *pe, uint16_t section_index, size_t start, size_t end);
void ppelib_section_insert_capacity(ppelib_file_t *pe, uint16_t section_index, size_t size, size_t offset);
void ppelib_section_resize(ppelib_file_t *pe, uint16_t section_index, size_t size);

EXPORT_SYM const char* ppelib_error();
EXPORT_SYM ppelib_file_t* ppelib_create();
EXPORT_SYM ppelib_file_t* ppelib_create_from_buffer(const uint8_t *buffer, size_t size);
EXPORT_SYM ppelib_file_t* ppelib_create_from_file(const char *filename);
EXPORT_SYM ppelib_header_t* ppelib_get_header(ppelib_file_t *pe);
EXPORT_SYM ppelib_resource_table_t* ppelib_get_resource_table(ppelib_file_t *pe);
EXPORT_SYM size_t ppelib_write_to_buffer(ppelib_file_t *pe, uint8_t *buffer, size_t buf_size);
EXPORT_SYM size_t ppelib_write_to_file(ppelib_file_t *pe, const char *filename);
EXPORT_SYM uint32_t ppelib_has_signature(ppelib_file_t *pe);
EXPORT_SYM void ppelib_destroy(ppelib_file_t *pe);
EXPORT_SYM void ppelib_fprint_pe_header(FILE *stream, const ppelib_header_t *header);
EXPORT_SYM void ppelib_free_header(ppelib_header_t *header);
EXPORT_SYM void ppelib_print_certificate_table(const ppelib_certificate_table_t *certificate_table);
EXPORT_SYM void ppelib_print_pe_header(const ppelib_header_t *header);
EXPORT_SYM void ppelib_print_resource_table(const ppelib_resource_table_t *resource_table);
EXPORT_SYM void ppelib_print_section(const ppelib_section_t *section);
EXPORT_SYM void ppelib_recalculate(ppelib_file_t *pe);
EXPORT_SYM void ppelib_set_header(ppelib_file_t *pe, ppelib_header_t *header);
EXPORT_SYM void ppelib_signature_remove(ppelib_file_t *pe);
EXPORT_SYM void ppelib_update_resource_table(ppelib_file_t *pe);

#endif /* PPELIB_INTERNAL_H_ */
