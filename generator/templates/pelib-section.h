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

#endif /* PELIB_SECTION_H */
