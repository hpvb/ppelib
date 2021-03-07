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

#ifndef PPELIB_RICH_TABLE_H_
#define PPELIB_RICH_TABLE_H_

#include <stdint.h>

#define RICH_MARKER 0x68636952 // Rich
#define DANS_MARKER 0x536E6144 // DanS

typedef struct rich_table_entry {
	uint16_t id;
	uint16_t build_number;
	uint32_t use_count;
} rich_table_entry_t;

typedef struct rich_table {
	size_t start;
	size_t end;

	size_t size;

	rich_table_entry_t* entries;
} rich_table_t;

#endif /* PPELIB_RICH_TABLE_H_ */
