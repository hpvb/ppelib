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

#ifndef INCLUDE_PELIB_H_
#define INCLUDE_PELIB_H_

#include <stddef.h>
#include <stdint.h>

#include <pelib/pelib-header.h>
#include <pelib/pelib-constants.h>

typedef void pelib_handle;

const char* pelib_error();

pelib_handle* pelib_create();
void pelib_destroy(pelib_handle* handle);

pelib_handle* pelib_create_from_buffer(uint8_t* buffer, size_t size);
pelib_handle* pelib_create_from_file(const char* filename);

size_t pelib_write_to_buffer(pelib_handle* handle, uint8_t* buffer, size_t size);
size_t pelib_write_to_file(pelib_handle* handle, const char* filename);

#endif /* INCLUDE_PELIB_H_ */
