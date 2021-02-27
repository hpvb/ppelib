#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <pelib/pelib-constants.h>

#include "pelib-error.h"
#include "pelib-header.h"

#include "utils.h"

{% from "print-field-macro.jinja" import print_field with context %}

/* serialize a pelib_header_t* back to the on-disk format */
/* When buffer is NULL only report how much we would write */
size_t serialize_pe_header(const pelib_header_t* header, uint8_t* buffer, size_t offset) {
  pelib_reset_error();

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

/* deserialize a buffer in on-disk format into a pelib_header_t */
/* Return value is the size of bytes consumed, if there is insufficient size returns 0 */
size_t deserialize_pe_header(const uint8_t* buffer, size_t offset, const size_t size, pelib_header_t* header) {
  pelib_reset_error();

  if (size - offset < {{sizes.common}}) {
	pelib_set_error("Buffer too small for common COFF headers.");
    return 0;
  }

  const uint8_t* buf = buffer + offset;
  const uint8_t* directories;
{% for field in common_fields %}
  header->{{field.name}} = read_{{field.type}}(buf + {{field.offset}});
{%- endfor %}

  if (header->{{pe_magic_field}} != PE32_MAGIC && header->{{pe_magic_field}} != PE32PLUS_MAGIC) {
	pelib_set_error("Unknown PE magic.");
    return 0;
  }

  if (header->{{pe_magic_field}} == PE32_MAGIC) {
    if (size - offset < {{sizes.total_pe}}) {
      pelib_set_error("Buffer too small for PE headers.");
      return 0;
    }
{%- for field in pe_fields %}
    header->{{field.name}} = read_{{field.type}}(buf + {{field.offset}});
{%- endfor %}

    directories = buf + {{sizes.total_pe}};
  }

  if (header->{{pe_magic_field}} == PE32PLUS_MAGIC) {
    if (size - offset < {{sizes.total_peplus}}) {
      pelib_set_error("Buffer too small for PE+ headers.");
      return 0;
    }
{%- for field in peplus_fields %}
    header->{{field.name}} = read_{{field.type}}(buf + {{field.offset}});
{%- endfor %}

    directories = buf + {{sizes.total_peplus}};
  }

  header->data_directories = malloc(header->{{pe_rvas_field}} * PE_HEADER_DATA_DIRECTORIES_SIZE);

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

void print_pe_header(const pelib_header_t* header) {
  pelib_reset_error();

{%- for field in common_fields %}
{{print_field(field, "header")|indent(2)}}
{%- endfor %}

  if (header->{{pe_magic_field}} != PE32_MAGIC && header->{{pe_magic_field}} != PE32PLUS_MAGIC) {
    return;
  }

  if (header->{{pe_magic_field}} == PE32_MAGIC) {
{%- for field in pe_fields %}
  {{print_field(field, "header")|indent(4)}}
{%- endfor %}
  }
  if (header->{{pe_magic_field}} == PE32PLUS_MAGIC) {
{%- for field in peplus_fields %}
  {{print_field(field, "header")|indent(4)}}
{%- endfor %}
  }
  printf("Data directories:\n");
  for (uint32_t i = 0; i < header->{{pe_rvas_field}}; ++i) {
    if (i < {{directories|length}}) {
      printf("%s: RVA: 0x%08X, size: %i\n", data_directory_names[i], header->data_directories[i].virtual_address, header->data_directories[i].size);
    } else {
      printf("Unknown: RVA: 0x%08X, size: %i\n", header->data_directories[i].virtual_address, header->data_directories[i].size);
    }
  }
}
