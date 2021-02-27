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

#include <stdint.h>

#include "utils.h"

uint8_t read_uint8_t(const uint8_t* buffer) {
        return *buffer;
}

void write_uint8_t(uint8_t* buffer, uint8_t val) {
        buffer[0] = val;
}

uint16_t read_uint16_t(const uint8_t* buffer) {
        return *(uint16_t*)buffer;
}

void write_uint16_t(uint8_t* buffer, uint16_t val) {
        ((uint16_t*)buffer)[0] = val;
}

uint32_t read_uint32_t(const uint8_t* buffer) {
        return *(uint32_t*)buffer;
}

void write_uint32_t(uint8_t* buffer, uint32_t val) {
        ((uint32_t*)buffer)[0] = val;
}

uint64_t read_uint64_t(const uint8_t* buffer) {
        return *(uint64_t*)buffer;
}

void write_uint64_t(uint8_t* buffer, uint64_t val) {
        ((uint64_t*)buffer)[0] = val;
}

const char* map_lookup(uint32_t value, const ppelib_map_entry_t* map) {
        const ppelib_map_entry_t* m = map;
        while (m->string) {
                if (m->value == value) {
                        return m->string;
                }
                ++m;
        }

        return NULL;
}
