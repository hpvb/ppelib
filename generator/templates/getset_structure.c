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

#include <string.h>
#include <stddef.h>
#include <inttypes.h>

#include <ppelib/ppelib-constants.h>
#include "ppe_error.h"

#include "utils.h"

#include "{{s.structure}}_private.h"

{% for field in s.fields -%}
{%- if field.getset_type == "section_name" %}
const char* ppelib_{{s.structure}}_get_{{field.struct_name}}(const {{s.structure}}_t* {{s.structure}}) {
	ppelib_reset_error();

	if (!{{s.structure}}) {
		ppelib_set_error("NULL pointer");
		return NULL;
	}

	return {{s.structure}}->{{field.struct_name}};
}

void ppelib_{{s.structure}}_set_{{field.struct_name}}({{s.structure}}_t* {{s.structure}}, const char value[9]) {
	ppelib_reset_error();

	if (!{{s.structure}}) {
		ppelib_set_error("NULL pointer");
		return;
	}

	memcpy({{s.structure}}->{{field.struct_name}}, value, 8);
	{{s.structure}}->{{field.struct_name}}[8] = 0;
}

{%- else %}
{{field.getset_type}} ppelib_{{s.structure}}_get_{{field.struct_name}}(const {{s.structure}}_t* {{s.structure}}) {
	ppelib_reset_error();
	if (!{{s.structure}}) {
		ppelib_set_error("NULL pointer");
		return 0;
	}
	return {{s.structure}}->{{field.struct_name}};
}
{% if field.set %}
void ppelib_{{s.structure}}_set_{{field.struct_name}}({{s.structure}}_t* {{s.structure}}, const {{field.getset_type}} value) {
	ppelib_reset_error();
	if (!{{s.structure}}) {
		ppelib_set_error("NULL pointer");
		return;
	}

{%- if field.range %}
{%- if 'start' in field.range and 'end' in field.range %}
	if (value < {{field.range.start}} || value > {{field.range.end}}) {
		ppelib_set_error("{{field.struct_name}}: value out of range ({{field.range.start}} - {{field.range.end}})");
{%- elif 'start' in field.range %}
	if (value < {{field.range.start}}) {
		ppelib_set_error("{{field.struct_name}}: value out of range (>= {{field.range.start}})");
{%- elif 'end' in field.range %}
	if (value > {{field.range.end}}) {
		ppelib_set_error("{{field.struct_name}}: value out of range (<= {{field.range.end}})");
{%- endif %}
		return;
	}
{%- endif %}
{%- if field.pe_type != field.getset_type %}
	if (header->magic == PE32_MAGIC) {
		if (value > {{s.max_sizes[field.pe_type]}}) {
			ppelib_set_error("{{field.struct_name}}: value out of range > {{s.max_sizes[field.pe_type]}}");
			return;
		}
	}
{%- endif %}
{%- if field.copy == true %}
	memset({{s.structure}}->{{field.struct_name}}, 0, sizeof({{s.structure}}));
	memcpy({{s.structure}}->{{field.struct_name}}, value, {{field.pe_size}});
{%- else %}
	{{s.structure}}->{{field.struct_name}} = value;
{%- endif %}
}
{% endif %}
{%- if field.format and field.format.enum %}
const char* ppelib_{{s.structure}}_get_{{field.struct_name}}_string(const {{s.structure}}_t* {{s.structure}}) {
	ppelib_reset_error();

	if (!{{s.structure}}) {
		ppelib_set_error("NULL pointer");
		return NULL;
	}
	return map_lookup({{s.structure}}->{{field.struct_name}}, {{field.format.enum}});
}
{% endif %}
{% endif %}
{% endfor -%}
