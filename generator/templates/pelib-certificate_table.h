#ifndef PELIB_CERTIFICATE_TABLE_H
#define PELIB_CERTIFICATE_TABLE_H

#include <stdint.h>
#include <stddef.h>

#include "pelib-header.h"

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

#endif /* PELIB_CERTIFICATE_TABLE_H */
