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

#ifndef PELIB_GENERATED_H_
#define PELIB_GENERATED_H_

#include <stdint.h>
#include <stddef.h>

#include <pelib/pelib-certificate_table.h>
#include <pelib/pelib-header.h>
#include <pelib/pelib-section.h>

size_t serialize_certificate_table(const pelib_certificate_table_t* certificate_table, uint8_t* buffer);
size_t deserialize_certificate_table(const uint8_t* buffer, pelib_header_t* header, const size_t size, pelib_certificate_table_t* certificate_table);

size_t serialize_pe_header(const pelib_header_t* header, uint8_t* buffer, size_t offset);
size_t deserialize_pe_header(const uint8_t* buffer, size_t offset, const size_t size, pelib_header_t* header);

size_t serialize_section(const pelib_section_t* section, uint8_t* buffer, size_t offset);
size_t deserialize_section(const uint8_t* buffer, size_t offset, const size_t size, pelib_section_t* section);

#endif /* PELIB_GENERATED_H_ */
