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

#ifndef PELIB_HEADER_H
#define PELIB_HEADER_H

#include <stdint.h>
#include <stddef.h>

enum data_directory_type {
{%- for d in directories %}
  {{d.name}} = {{loop.index - 1}},
{%- endfor %}
};

static const char* const data_directory_names[] = {
{%- for d in directories %}
  "{{d.human_name}}",
{%- endfor %}
};

typedef struct pelib_data_directory {
  uint32_t virtual_address;
  uint32_t size;
} pelib_data_directory_t;

typedef struct pelib_header {
{%- for f in fields %}
{%- if 'peplus_type' in f %}
  {{f.peplus_type}} {{f.name}};
{%- else %}
  {{f.pe_type}} {{f.name}};
{%- endif %}
{%- endfor %}

  pelib_data_directory_t* data_directories;
} pelib_header_t;

void print_pe_header(const pelib_header_t* header);

#endif /* PELIB_HEADER_H */
