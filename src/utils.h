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

#ifndef PPELIB_UTILS_H
#define PPELIB_UTILS_H

#include <inttypes.h>
#include <stddef.h>

#include <ppelib/ppelib-constants.h>

#define HIGH_BIT32 ((uint32_t)(1) << 31)
#define CHECK_BIT(var,val) ((var) & (val))
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X ,Y) (((X) > (Y)) ? (X) : (Y))
#define TO_NEAREST(num, size) ((num == 0) ? 0 : (((num) + (size) - 1u - ((num) + (size) - 1u) % (size))))
#define SWAP(x, y) do { uint8_t swap_temp[sizeof(x)]; memcpy(swap_temp, x, sizeof(x)); x = y; memcpy(y, swap_temp, sizeof(x)); }  while (0)

uint8_t read_uint8_t(const uint8_t *buffer);
void write_uint8_t(uint8_t *buffer, uint8_t val);
uint16_t read_uint16_t(const uint8_t *buffer);
void write_uint16_t(uint8_t *buffer, uint16_t val);
uint32_t read_uint32_t(const uint8_t *buffer);
void write_uint32_t(uint8_t *buffer, uint32_t val);
uint64_t read_uint64_t(const uint8_t *buffer);
void write_uint64_t(uint8_t *buffer, uint64_t val);

uint16_t buffer_excise(uint8_t **buffer, size_t size, size_t start, size_t end);

EXPORT_SYM const char* map_lookup(uint32_t value, const ppelib_map_entry_t *map);

#endif /* PPELIB_UTILS_H */
