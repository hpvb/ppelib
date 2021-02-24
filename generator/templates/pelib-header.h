#ifndef PELIB_HEADER_H
#define PELIB_HEADER_H

#include <stdint.h>
#include <stddef.h>

#include "constants.h"

enum data_directory {
{%- for d in directories %}
  {{d.name}} = {{loop.index - 1}},
{%- endfor %}
};

static const char* data_directory_names[] = {
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

  pelib_data_directory_t data_directories[{{directories|length}}];
  pelib_data_directory_t* unknown_data_directories;
} pelib_header_t;

uint32_t serialize_pe_header(const pelib_header_t* header, uint8_t* buffer);
uint32_t deserialize_pe_header(const uint8_t* buffer, const size_t size, pelib_header_t* header);
void print_pe_header(const pelib_header_t* header);

#endif /* PELIB_HEADER_H */
