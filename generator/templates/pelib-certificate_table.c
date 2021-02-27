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

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <pelib/pelib-constants.h>
#include <pelib/pelib-certificate_table.h>
#include <pelib/pelib-header.h>

#include "pelib-error.h"
#include "export.h"
#include "utils.h"

{% from "print-field-macro.jinja" import print_field with context %}

/* serialize a pelib_certificate_table_t* back to the on-disk format */
/* When buffer is NULL only report how much we would write */
size_t serialize_certificate_table(const pelib_certificate_table_t* certificate_table, uint8_t* buffer) {
  pelib_reset_error();

  size_t size = 0;
  size_t offset = certificate_table->offset;

  for (uint32_t i = 0; i < certificate_table->size; ++i) {
    size += certificate_table->certificates[i].length;

    if (buffer) {
{%- for field in fields %}
{%- if 'format' in field and 'variable_size' in field.format %}
      memcpy(buffer + offset + {{field.offset}}, certificate_table->certificates[i].{{field.name}}, certificate_table->certificates[i].{{length_field}} - 8);
{%- else %}
      write_{{field.pe_type}}(buffer + offset + {{field.offset}}, certificate_table->certificates[i].{{field.name}});
{%- endif %}
{%- endfor %}
    }

    offset = TO_NEAREST(offset + certificate_table->certificates[i].{{length_field}}, 8);
  }

  return size + certificate_table->offset;
}

/* deserialize a buffer in on-disk format into a pelib_certificate_table_t */
/* Return value is the size of bytes consumed, if there is insufficient size returns 0 */
size_t deserialize_certificate_table(const uint8_t* buffer, pelib_header_t* header, const size_t size, pelib_certificate_table_t* certificate_table) {
  pelib_reset_error();

  size_t table_offset = header->data_directories[DIR_CERTIFICATE_TABLE].virtual_address;
  size_t table_size = header->data_directories[DIR_CERTIFICATE_TABLE].size;

  memset(certificate_table, 0, sizeof(pelib_certificate_table_t));

  if (! table_offset || ! table_size) {
    pelib_set_error("No certificate table found.");
    return 0;
  }

  if (table_offset + table_size > size) {
    pelib_set_error("Buffer too small for table.");
    return 0;
  }

  size_t certificate_count = 0;
  size_t offset = table_offset;
  size_t max_offset = table_offset + table_size;

  while (offset < max_offset) {
    size_t i = certificate_count;
    certificate_count++;

    certificate_table->certificates = realloc(certificate_table->certificates, sizeof(pelib_certificate_t) * certificate_count);
{%- for field in fields %}
{%- if 'format' in field and 'variable_size' in field.format %}
{%- else %}
    certificate_table->certificates[i].{{field.name}} = read_{{field.pe_type}}(buffer + offset + {{field.offset}});
{%- endif %}
{%- endfor %}

    if (offset + certificate_table->certificates[i].{{length_field}} > max_offset) {
      pelib_set_error("Buffer too small for table.");
      return 0;
    }

    certificate_table->certificates[i].certificate = malloc(certificate_table->certificates[i].{{length_field}});
    memcpy(certificate_table->certificates[i].certificate, buffer + offset + 8, certificate_table->certificates[i].{{length_field}} - 8);

    offset = TO_NEAREST(offset + certificate_table->certificates[i].{{length_field}}, 8);
  }

  certificate_table->size = certificate_count;
  certificate_table->offset = table_offset;

  return max_offset;
}

EXPORT_SYM void print_certificate_table(const pelib_certificate_table_t* certificate_table) {
  pelib_reset_error();

  printf("Certificate table: \n");
  for (size_t i = 0; i < certificate_table->size; ++i) {
    printf("Certificate: %li\n", i);
    pelib_certificate_t* certificate = &certificate_table->certificates[i];

{%- for field in fields -%}
  {{print_field(field, "certificate")|indent(4)}}
{%- endfor %}
  }
}
