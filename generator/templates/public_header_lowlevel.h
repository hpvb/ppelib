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

#ifndef PPELIB_{{s.structure|upper}}_LOWLEVEL_H_
#define PPELIB_{{s.structure|upper}}_LOWLEVEL_H_

#include <inttypes.h>
#include <stdio.h>
#include <stddef.h>

#include <ppelib/ppelib-{{s.structure}}.h>

size_t ppelib_{{s.structure}}_serialize(const ppelib_{{s.structure}}* {{s.structure}}, uint8_t* buffer, const size_t offset);
void ppelib_{{s.structure}}_fprint(FILE* stream, const ppelib_{{s.structure}}* {{s.structure}});
void ppelib_{{s.structure}}_print(const ppelib_{{s.structure}}* {{s.structure}});

#endif /* PPELIB_{{s.structure|upper}}_LOWLEVEL_H_  */
