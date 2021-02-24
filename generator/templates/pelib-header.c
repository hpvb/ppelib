#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "constants.h"
#include "utils.h"
#include "pelib-header.h"

/* serialize a pelib_header_t* back to the on-disk format */
/* When buffer is NULL only report how much we would write */
uint32_t serialize_pe_header(const pelib_header_t* header, uint8_t* buffer) {
  if (! buffer) {
    goto end;
  }

  if (header->{{pe_magic_field}} != PE32_MAGIC && header->{{pe_magic_field}} != PE32PLUS_MAGIC) {
    goto end;
  }

  uint8_t* directories;
{% for field in common_fields %}
  write_{{field.type}}(buffer + {{field.offset}}, header->{{field.name}});
{%- endfor %}

  if (header->{{pe_magic_field}} == PE32_MAGIC) {
{%- for field in pe_fields %}
    write_{{field.type}}(buffer + {{field.offset}}, header->{{field.name}});
{%- endfor %}

    directories = buffer + {{sizes.total_pe}};
  }

  if (header->{{pe_magic_field}} == PE32PLUS_MAGIC) {
{%- for field in peplus_fields %}
    write_{{field.type}}(buffer + {{field.offset}}, header->{{field.name}});
{%- endfor %}

    directories = buffer + {{sizes.total_peplus}};
  }

  for (uint32_t i = 0; i < header->{{pe_rvas_field}}; ++i) {
    if (i < {{directories|length}}) {
      write_uint32_t(directories, header->data_directories[i].virtual_address);
      write_uint32_t(directories + sizeof(uint32_t), header->data_directories[i].size);
    } else {
      write_uint32_t(directories, header->unknown_data_directories[i - {{directories|length - 1 }}].virtual_address);
      write_uint32_t(directories + sizeof(uint32_t), header->unknown_data_directories[i - {{directories|length - 1 }}].size);
    }
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
uint32_t deserialize_pe_header(const uint8_t* buffer, const size_t size, pelib_header_t* header) {
  if (size < {{sizes.common}}) {
    return 0;
  }

  const uint8_t* directories;
{% for field in common_fields %}
  header->{{field.name}} = read_{{field.type}}(buffer + {{field.offset}});
{%- endfor %}

  if (header->{{pe_magic_field}} != PE32_MAGIC && header->{{pe_magic_field}} != PE32PLUS_MAGIC) {
    return 0;
  }

  if (header->{{pe_magic_field}} == PE32_MAGIC) {
    if (size < {{sizes.total_pe}}) {
      return 0;
    }
{%- for field in pe_fields %}
    header->{{field.name}} = read_{{field.type}}(buffer + {{field.offset}});
{%- endfor %}

    directories = buffer + {{sizes.total_pe}};
  }

  if (header->{{pe_magic_field}} == PE32PLUS_MAGIC) {
    if (size < {{sizes.total_peplus}}) {
      return 0;
    }
{%- for field in peplus_fields %}
    header->{{field.name}} = read_{{field.type}}(buffer + {{field.offset}});
{%- endfor %}

    directories = buffer + {{sizes.total_peplus}};
  }

  if (header->{{pe_rvas_field}} > {{directories|length}}) {
    header->unknown_data_directories = malloc((header->{{pe_rvas_field}} - {{directories|length}}) * PE_HEADER_DATA_DIRECTORIES_SIZE);
  }

  for (uint32_t i = 0; i < header->{{pe_rvas_field}}; ++i) {
    if (i < {{directories|length}}) {
      header->data_directories[i].virtual_address = read_uint32_t(directories);
      header->data_directories[i].size = read_uint32_t(directories + sizeof(uint32_t));
    } else {
      header->unknown_data_directories[i].virtual_address = read_uint32_t(directories);
      header->unknown_data_directories[i].size = read_uint32_t(directories + sizeof(uint32_t));
    }
    directories += PE_HEADER_DATA_DIRECTORIES_SIZE;
  }
}

{%- macro print_field(field) -%}
{%- if 'format' in field %}
{%- if 'enum' in field['format'] %}
printf("{{field.human_name}}: %s\n", map_lookup(header->{{field.name}}, {{field['format']['enum']}}));
{%- elif 'hex' in field['format'] %}
printf("{{field.human_name}}: 0x%08X\n", header->{{field.name}});
{%- elif 'bitfield' in field['format'] %}
printf("{{field.human_name}}: ");
map_entry_t* {{field.name}}_map_i = {{field['format']['bitfield']}};
while ({{field.name}}_map_i->string) {
  if (CHECK_BIT(header->{{field.name}}, {{field.name}}_map_i->value)) {
    printf("%s ", {{field.name}}_map_i->string);
  }
  ++{{field.name}}_map_i;
}
printf("\n");
{%- endif %}
{%- else %}
printf("{{field.human_name}}: %li\n", header->{{field.name}});
{%- endif %}
{%- endmacro %}

void print_pe_header(const pelib_header_t* header) {
{%- for field in common_fields -%}
{{print_field(field)|indent(2)}}
{%- endfor %}

  if (header->{{pe_magic_field}} != PE32_MAGIC && header->{{pe_magic_field}} != PE32PLUS_MAGIC) {
    return;
  }

  if (header->{{pe_magic_field}} == PE32_MAGIC) {
{%- for field in pe_fields -%}
  {{print_field(field)|indent(4)}}
{%- endfor %}
  }
  if (header->{{pe_magic_field}} == PE32PLUS_MAGIC) {
{%- for field in peplus_fields -%}
  {{print_field(field)|indent(4)}}
{%- endfor %}
  }
  printf("Data directories:\n");
  for (uint32_t i = 0; i < header->{{pe_rvas_field}}; ++i) {
    if (i < {{directories|length}}) {
      printf("%s: RVA: 0x%08lX, size: %li\n", data_directory_names[i], header->data_directories[i].virtual_address, header->data_directories[i].size);
    } else {
      printf("Unknown%i: RVA: 0x%08lX, size: %li\n", i - ({{directories|length}} - 1), header->unknown_data_directories[i - ({{directories|length}} - 1)].virtual_address, header->unknown_data_directories[i - ({{directories|length}} - 1)].size);
    }
  }
}
