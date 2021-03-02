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

#ifndef PPELIB_LOW_LEVEL_H_
#define PPELIB_LOW_LEVEL_H_

#include <ppelib/ppelib.h>

ppelib_header_t* ppelib_get_header(ppelib_handle *handle);
ppelib_resource_table_t* ppelib_get_resource_table(ppelib_handle *handle);
void ppelib_free_header(ppelib_header_t *header);
void ppelib_free_resource_directory_table(ppelib_resource_table_t *table);
void ppelib_recalculate(ppelib_handle *file);
void ppelib_set_header(ppelib_handle *handle, ppelib_header_t *header);

#endif /* PPELIB_LOW_LEVEL_H_ */
