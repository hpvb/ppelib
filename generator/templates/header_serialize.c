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

#include <stddef.h>
#include <inttypes.h>

#include <ppelib/ppelib-constants.h>
#include "ppe_error.h"

#include "utils.h"

#include "{{s.structure}}_private.h"

size_t ppelib_serialize_{{s.structure}}(const {{s.structure}}_t* {{s.structure}}, uint8_t* buffer, const size_t offset) {
	ppelib_reset_error();

	if (! buffer) {
		goto out;
	}

	{% for field in s.fields -%}
	{%- if field.common -%}
	write_{{field.pe_type}}(buffer + offset + {{field.pe_offset}}, {{s.structure}}->{{field.struct_name}});
	{% endif -%}
	{% endfor %}

	if ({{s.structure}}->magic == PE32_MAGIC) {
		{% for field in s.fields -%}
		{%- if not field.common -%}
		write_{{field.pe_type}}(buffer + offset + {{field.pe_offset}}, ({{field.pe_type}}){{s.structure}}->{{field.struct_name}});
		{% endif -%}
		{%- endfor %}
	} else if ({{s.structure}}-> magic == PE32PLUS_MAGIC) {
		{% for field in s.fields -%}
		{%- if not field.common and not field.pe_only-%}
		{%- if field.peplus_type -%}
		write_{{field.peplus_type}}(buffer + offset + {{field.peplus_offset}}, {{s.structure}}->{{field.struct_name}});
		{% else -%}
		write_{{field.pe_type}}(buffer + offset + {{field.peplus_offset}}, {{s.structure}}->{{field.struct_name}});
		{% endif -%}
		{% endif -%}
		{% endfor %}
	} else {
		ppelib_error("Unknown magic type");
		return 0;
	}

	out:
	switch ({{s.structure}}->magic) {
		case PE32_MAGIC:
			return {{s.pe_size}};
		case PE32PLUS_MAGIC:
			return {{s.peplus_size}};
		default:
			return 0;
	}
}

