#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <pelib/pelib_constants.h>
#include "pelib-section.h"

#include "utils.h"

{% from "print-field-macro.jinja" import print_field with context %}

/* serialize a pelib_section_t* back to the on-disk format */
/* When buffer is NULL only report how much we would write */
size_t serialize_section(const pelib_section_t* section, uint8_t* buffer, size_t offset) {
  size_t data_size = MIN(section->{{virtualsize_field}}, section->{{rawsize_field}});

  if (! buffer) {
    goto end;
  }

  uint8_t* section_header = buffer + offset;

{%- for field in fields %}
{%- if 'format' in field and 'string' in field.format %}
  memcpy(section_header + {{field.offset}}, section->{{field.name}}, {{field.pe_size}});
{%- else %}
  write_{{field.pe_type}}(section_header + {{field.offset}}, section->{{field.name}});
{%- endif %}
{%- endfor %}

  if (data_size) {
    memcpy(buffer + section->{{pointer_field}}, section->contents, data_size);
  }

  end:
  if (section->{{pointer_field}} > offset) {
    return section->{{pointer_field}} + data_size;
  } else {
    return PE_SECTION_HEADER_SIZE;
  }
}

/* deserialize a buffer in on-disk format into a pelib_section_t */
/* Return value is the size of bytes consumed, if there is insufficient size returns 0 */
size_t deserialize_section(const uint8_t* buffer, size_t offset, const size_t size, pelib_section_t* section) {
  if (size - offset < PE_SECTION_HEADER_SIZE) {
    return 0;
  }

  memset(section, 0, sizeof(pelib_section_t));
  const uint8_t* section_header = buffer + offset;

{%- for field in fields %}
{%- if 'format' in field and 'string' in field.format %}
  memcpy(section->{{field.name}}, section_header + {{field.offset}}, {{field.pe_size}});
{%- else %}
  section->{{field.name}} = read_{{field.pe_type}}(section_header + {{field.offset}});
{%- endif %}
{%- endfor %}

  size_t data_size = MIN(section->{{virtualsize_field}}, section->{{rawsize_field}});

  if (section->{{pointer_field}} + data_size > size) {
    return 0;
  }

  if (data_size) {
    section->contents = malloc(data_size);
    memcpy(section->contents, buffer + section->{{pointer_field}}, data_size);
  }

  if (section->{{pointer_field}} > offset) {
    return section->{{pointer_field}} + data_size;
  } else {
    return PE_SECTION_HEADER_SIZE;
  }
}

void print_section(const pelib_section_t* section) {
{%- for field in fields -%}
{{print_field(field, "section")|indent(2)}}
{%- endfor %}
}
