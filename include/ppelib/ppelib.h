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
#include <stdint.h>

#include <ppelib/ppelib-constants.h>
#include <ppelib/ppelib-certificate_table.h>
#include <ppelib/ppelib-header.h>
#include <ppelib/ppelib-section.h>

typedef void ppelib_handle;

const char* ppelib_error();

ppelib_handle* ppelib_create();
void ppelib_destroy(ppelib_handle* handle);

ppelib_handle* ppelib_create_from_buffer(uint8_t* buffer, size_t size);
ppelib_handle* ppelib_create_from_file(const char* filename);

size_t ppelib_write_to_buffer(ppelib_handle* handle, uint8_t* buffer, size_t size);
size_t ppelib_write_to_file(ppelib_handle* handle, const char* filename);

#endif /* PPELIB_H_ */
