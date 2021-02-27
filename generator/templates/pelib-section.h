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

#ifndef PELIB_SECTION_H
#define PELIB_SECTION_H

#include <stdint.h>
#include <stddef.h>

typedef struct pelib_section {
{%- for f in fields %}
{%- if 'format' in f and 'string' in f.format %}
  char {{f.name}}[{{f.pe_size + 1}}];
{%- else %}
  {{f.pe_type}} {{f.name}};
{%- endif %}
{%- endfor %}
  uint8_t* contents;
} pelib_section_t;

void print_section(const pelib_section_t* section);

#endif /* PELIB_SECTION_H */
