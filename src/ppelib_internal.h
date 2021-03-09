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

#include "main.h"
#include "platform.h"

#include "generated/dos_header_private.h"
#include "generated/section_private.h"
#include "generated/vlv_signature_private.h"

#include "dos_header/rich_table.h"

section_t *section_find_by_physical_address(ppelib_file_t *pe, size_t address);
section_t *section_find_by_virtual_address(ppelib_file_t *pe, size_t va);
size_t section_rva_to_offset(const section_t *section, size_t rva);
void *section_rva_to_pointer(const section_t *section, size_t rva);

void parse_dos_stub(dos_header_t *dos_header);
void update_dos_stub(dos_header_t *dos_header);

uint8_t parse_vlv_signature(uint8_t *buffer, size_t size, vlv_signature_t *vlv_signature);
uint8_t parse_rich_table(uint8_t *buffer, size_t size, rich_table_t *rich_table);

EXPORT_SYM void ppelib_recalculate(ppelib_file_t *pe);

const char *string_table_get(string_table_t *string_table, size_t offset);
void string_table_free(string_table_t *string_table);
void parse_string_table(const uint8_t *buffer, size_t size, size_t offset, string_table_t *string_table);

void parse_import_table(const section_t *section, size_t offset, import_table_t *import_table);
void import_table_free(import_table_t *import_table);
#endif /* PPELIB_INTERNAL_H_ */
