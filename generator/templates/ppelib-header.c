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
#include <stdint.h>
#include <stdio.h>

#include <ppelib/ppelib-constants.h>
#include <ppelib/ppelib-header.h>
#include "ppelib-error.h"
#include "export.h"
#include "utils.h"

{% from "print-field-macro.jinja" import print_field with context %}

size_t serialize_pe_header(const ppelib_header_t* header, uint8_t* buffer, size_t offset) {
  ppelib_reset_error();

  if (! buffer) {
    goto end;
  }

  if (header->{{pe_magic_field}} != PE32_MAGIC && header->{{pe_magic_field}} != PE32PLUS_MAGIC) {
    goto end;
  }

  uint8_t* buf = buffer + offset;
  uint8_t* directories;
{% for field in common_fields %}
  write_{{field.type}}(buf + {{field.offset}}, header->{{field.name}});
{%- endfor %}

  if (header->{{pe_magic_field}} == PE32_MAGIC) {
{%- for field in pe_fields %}
    write_{{field.type}}(buf + {{field.offset}}, header->{{field.name}});
{%- endfor %}

    directories = buf + {{sizes.total_pe}};
  }

  if (header->{{pe_magic_field}} == PE32PLUS_MAGIC) {
{%- for field in peplus_fields %}
    write_{{field.type}}(buf + {{field.offset}}, header->{{field.name}});
{%- endfor %}

    directories = buf + {{sizes.total_peplus}};
  }

  for (uint32_t i = 0; i < header->{{pe_rvas_field}}; ++i) {
    write_uint32_t(directories, header->data_directories[i].virtual_address);
    write_uint32_t(directories + sizeof(uint32_t), header->data_directories[i].size);
    directories += PE_HEADER_DATA_DIRECTORIES_SIZE;
  }

  end:
  switch (header->{{pe_magic_field}}) {
    case PE32_MAGIC:
      return {{sizes.total_pe}} + (header->{{pe_rvas_field}} * PE_HEADER_DATA_DIRECTORIES_SIZE);
    case PE32PLUS_MAGIC:
      return {{sizes.total_peplus}} + (header->{{pe_rvas_field}} * PE_HEADER_DATA_DIRECTORIES_SIZE);
    default:
      return 0;
  }
}

size_t deserialize_pe_header(const uint8_t* buffer, size_t offset, const size_t size, ppelib_header_t* header) {
  ppelib_reset_error();

  if (size - offset < {{sizes.common}}) {
	ppelib_set_error("Buffer too small for common COFF headers.");
    return 0;
  }

  const uint8_t* buf = buffer + offset;
  const uint8_t* directories;
{% for field in common_fields %}
  header->{{field.name}} = read_{{field.type}}(buf + {{field.offset}});
{%- endfor %}

  if (header->{{pe_magic_field}} != PE32_MAGIC && header->{{pe_magic_field}} != PE32PLUS_MAGIC) {
	ppelib_set_error("Unknown PE magic.");
    return 0;
  }

  if (header->{{pe_magic_field}} == PE32_MAGIC) {
    if (size - offset < {{sizes.total_pe}}) {
      ppelib_set_error("Buffer too small for PE headers.");
      return 0;
    }
{%- for field in pe_fields %}
    header->{{field.name}} = read_{{field.type}}(buf + {{field.offset}});
{%- endfor %}

    if (size < {{sizes.total_pe}} + offset + (header->{{pe_rvas_field}} * PE_HEADER_DATA_DIRECTORIES_SIZE)) {
      ppelib_set_error("Buffer too small for directory entries.");
      return 0;
    }
    directories = buf + {{sizes.total_pe}};
  }

  if (header->{{pe_magic_field}} == PE32PLUS_MAGIC) {
    if (size - offset < {{sizes.total_peplus}}) {
      ppelib_set_error("Buffer too small for PE+ headers.");
      return 0;
    }
{%- for field in peplus_fields %}
    header->{{field.name}} = read_{{field.type}}(buf + {{field.offset}});
{%- endfor %}

	if (size < {{sizes.total_peplus}} + offset + (header->{{pe_rvas_field}} * PE_HEADER_DATA_DIRECTORIES_SIZE)) {
      ppelib_set_error("Buffer too small for directory entries.");
      return 0;
	}
    directories = buf + {{sizes.total_peplus}};
  }

  if (header->{{pe_rvas_field}} > header->{{pe_rvas_field}} * PE_HEADER_DATA_DIRECTORIES_SIZE) {
    ppelib_set_error("Too many directory entries");
    return 0;
  }

  header->data_directories = malloc(header->{{pe_rvas_field}} * PE_HEADER_DATA_DIRECTORIES_SIZE);
  if (!header->data_directories) {
	ppelib_set_error("Failed to allocate data directories.");
	return 0;
  }

  for (uint32_t i = 0; i < header->{{pe_rvas_field}}; ++i) {
    header->data_directories[i].virtual_address = read_uint32_t(directories);
    header->data_directories[i].size = read_uint32_t(directories + sizeof(uint32_t));
    directories += PE_HEADER_DATA_DIRECTORIES_SIZE;
  }

  switch (header->{{pe_magic_field}}) {
    case PE32_MAGIC:
      return {{sizes.total_pe}} + (header->{{pe_rvas_field}} * PE_HEADER_DATA_DIRECTORIES_SIZE);
    case PE32PLUS_MAGIC:
      return {{sizes.total_peplus}} + (header->{{pe_rvas_field}} * PE_HEADER_DATA_DIRECTORIES_SIZE);
    default:
      return 0;
  }
}

EXPORT_SYM void ppelib_fprint_pe_header(FILE* stream, const ppelib_header_t* header) {
  ppelib_reset_error();

{%- for field in common_fields %}
{{print_field(field, "header", "stream")|indent(2)}}
{%- endfor %}

  if (header->{{pe_magic_field}} != PE32_MAGIC && header->{{pe_magic_field}} != PE32PLUS_MAGIC) {
    return;
  }

  if (header->{{pe_magic_field}} == PE32_MAGIC) {
{%- for field in pe_fields %}
  {{print_field(field, "header", "stream")|indent(4)}}
{%- endfor %}
  }
  if (header->{{pe_magic_field}} == PE32PLUS_MAGIC) {
{%- for field in peplus_fields %}
  {{print_field(field, "header", "stream")|indent(4)}}
{%- endfor %}
  }
  fprintf(stream, "Data directories:\n");
  for (uint32_t i = 0; i < header->{{pe_rvas_field}}; ++i) {
    if (i < {{directories|length}}) {
      fprintf(stream, "%s: RVA: 0x%08X, size: %i\n", ppelib_data_directory_names[i], header->data_directories[i].virtual_address, header->data_directories[i].size);
    } else {
      fprintf(stream, "Unknown: RVA: 0x%08X, size: %i\n", header->data_directories[i].virtual_address, header->data_directories[i].size);
    }
  }
}

EXPORT_SYM void ppelib_print_pe_header(const ppelib_header_t* header) {
  ppelib_fprint_pe_header(stdout, header);
}
