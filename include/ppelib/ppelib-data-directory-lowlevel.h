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

#ifndef PPELIB_DATA_DIRECTORY_LOWLEVEL_H_
#define PPELIB_DATA_DIRECTORY_LOWLEVEL_H_

#include <stddef.h>
#include <inttypes.h>

#include <ppelib/ppelib-data-directory.h>

size_t ppelib_data_directory_serialize(const ppelib_data_directory *data_directory, uint8_t *buffer,
		const size_t offset);

void ppelib_data_directory_fprint(FILE *stream, const ppelib_data_directory *data_directory);
void ppelib_data_directory_print(const ppelib_data_directory *data_directory);

#endif /* PPELIB_DATA_DIRECTORY_LOWLEVEL_H_ */
