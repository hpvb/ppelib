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

#ifndef PELIB_CERTIFICATE_TABLE_H
#define PELIB_CERTIFICATE_TABLE_H

#include <stdint.h>
#include <stddef.h>

typedef struct pelib_certificate {
{%- for f in fields %}
{%- if 'format' in f and 'string' in f.format %}
  uint8_t {{f.name}}[{{f.pe_size + 1}}];
{%- else %}
  {{f.pe_type}} {{f.name}};
{%- endif %}
{%- endfor %}
} pelib_certificate_t;

typedef struct pelib_certificate_table {
  size_t size;
  size_t offset;
  pelib_certificate_t* certificates;
} pelib_certificate_table_t;

void print_certificate_table(const pelib_certificate_table_t* certificate_table);

#endif /* PELIB_CERTIFICATE_TABLE_H */
