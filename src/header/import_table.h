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

#ifndef PPELIB_IMPORT_TABLE_H_
#define PPELIB_IMPORT_TABLE_H_

#include "generated/import_directory_table_private.h"

typedef struct import_table {
	size_t size;

	import_directory_table_t *import_directory_tables;
} import_table_t;

#endif /* PPELIB_IMPORT_TABLE_H_ */
