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

#ifndef PPELIB_GENERATED_H_
#define PPELIB_GENERATED_H_

#include <stdint.h>
#include <stddef.h>

#include <ppelib/ppelib-certificate_table.h>
#include <ppelib/ppelib-header.h>
#include <ppelib/ppelib-section.h>

size_t serialize_certificate_table(const ppelib_certificate_table_t* certificate_table, uint8_t* buffer);
size_t deserialize_certificate_table(const uint8_t* buffer, ppelib_header_t* header, const size_t size, ppelib_certificate_table_t* certificate_table);

size_t serialize_pe_header(const ppelib_header_t* header, uint8_t* buffer, size_t offset);
size_t deserialize_pe_header(const uint8_t* buffer, size_t offset, const size_t size, ppelib_header_t* header);

size_t serialize_section(const ppelib_section_t* section, uint8_t* buffer, size_t offset);
size_t deserialize_section(const uint8_t* buffer, size_t offset, const size_t size, ppelib_section_t* section);

#endif /* PPELIB_GENERATED_H_ */
