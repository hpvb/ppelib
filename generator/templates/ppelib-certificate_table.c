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

#include <stdlib.h>
#include <stddef.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "ppelib-internal.h"

{% from "print-field-macro.jinja" import print_field with context %}

size_t serialize_certificate_table(const ppelib_certificate_table_t* certificate_table, uint8_t* buffer) {
  ppelib_reset_error();

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

size_t deserialize_certificate_table(const uint8_t* buffer, ppelib_header_t* header, const size_t size, ppelib_certificate_table_t* certificate_table) {
  ppelib_reset_error();

  size_t table_offset = header->data_directories[DIR_CERTIFICATE_TABLE].virtual_address;
  size_t table_size = header->data_directories[DIR_CERTIFICATE_TABLE].size;

  memset(certificate_table, 0, sizeof(ppelib_certificate_table_t));

  if (! table_offset || ! table_size) {
    ppelib_set_error("No certificate table found.");
    return 0;
  }

  if (table_offset + table_size > size) {
    ppelib_set_error("Buffer too small for table.");
    return 0;
  }

  size_t offset = table_offset;
  size_t prev_offset = 0;
  size_t max_offset = table_offset + table_size;

  while (offset < max_offset) {
	prev_offset = offset;
    size_t i = certificate_table->size;

    if (offset + {{length}} > size){
    	ppelib_set_error("Not enough space for certificate entry");
    	return 0;
    }

    certificate_table->size++;
    certificate_table->certificates = realloc(certificate_table->certificates, sizeof(ppelib_certificate_t) * certificate_table->size);

    {%- for field in fields %}
{%- if 'format' in field and 'variable_size' in field.format %}
{%- else %}
    certificate_table->certificates[i].{{field.name}} = read_{{field.pe_type}}(buffer + offset + {{field.offset}});
{%- endif %}
{%- endfor %}

    if (certificate_table->certificates[i].length < 8) {
	  ppelib_set_error("Certificate too small");
	  certificate_table->certificates[i].certificate = NULL;
	  return 0;
    }

    if (offset + certificate_table->certificates[i].length > size) {
      ppelib_set_error("Buffer too small for table.");
	  certificate_table->certificates[i].certificate = NULL;
      return 0;
    }

    certificate_table->certificates[i].certificate = malloc(certificate_table->certificates[i].{{length_field}});
    if (!certificate_table->certificates[i].certificate){
      ppelib_set_error("Unable to allocate certificate");
      return 0;
    }
    memcpy(certificate_table->certificates[i].certificate, buffer + offset + 8, certificate_table->certificates[i].{{length_field}} - 8);

    offset = TO_NEAREST(offset + certificate_table->certificates[i].{{length_field}}, 8);
    if (offset < prev_offset) {
    	ppelib_set_error("Wrong length in certificate table");
    	return 0;
    }
  }

  if (table_offset > UINT32_MAX) {
	  ppelib_set_error("Certificate table offset out of range");
	  return 0;
  }

  certificate_table->offset = (uint32_t) table_offset;

  return max_offset;
}

EXPORT_SYM void ppelib_print_certificate_table(const ppelib_certificate_table_t* certificate_table) {
  ppelib_reset_error();

  printf("Certificate table: \n");
  for (uint32_t i = 0; i < certificate_table->size; ++i) {
    printf("Certificate: %u\n", i);
    ppelib_certificate_t* certificate = &certificate_table->certificates[i];

{%- for field in fields -%}
  {{print_field(field, "certificate")|indent(4)}}
{%- endfor %}
  }
}

void ppelib_free_certificate_table(ppelib_certificate_table_t* certificate_table) {
	if (!certificate_table->size) {
		return;
	}

	for (size_t i = 0; i < certificate_table->size; ++i) {
		free(certificate_table->certificates[i].certificate);
	}
	free(certificate_table->certificates);

	certificate_table->size = 0;
}
