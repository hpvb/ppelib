#ifndef PELIB_SECTION_H
#define PELIB_SECTION_H

#include <stdint.h>
#include <stddef.h>

typedef struct pelib_section {
{%- for f in fields %}
{%- if 'format' in f and 'string' in f.format %}
  uint8_t {{f.name}}[{{f.pe_size + 1}}];
{%- else %}
  {{f.pe_type}} {{f.name}};
{%- endif %}
{%- endfor %}
  uint8_t* contents;
} pelib_section_t;

size_t serialize_section(const pelib_section_t* section, uint8_t* buffer, size_t offset);
size_t deserialize_section(const uint8_t* buffer, size_t offset, const size_t size, pelib_section_t* section);
void print_section(const pelib_section_t* section);

#endif /* PELIB_SECTION_H */
