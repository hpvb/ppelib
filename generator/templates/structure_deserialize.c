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

#include "generated/{{s.structure}}_private.h"

size_t ppelib_{{s.structure}}_deserialize(const uint8_t* buffer, const size_t size, const size_t offset, {{s.structure}}_t* {{s.structure}}) {
	ppelib_reset_error();

	if (offset + {{s.common_size}} > size) {
		ppelib_set_error("Not enough space for {{s.structure}} headers");
		return 0;
	}

	{{s.structure}}->modified = 0;

	{% for field in s.fields -%}
	{% if field.getset_type == "string_name" -%}
	memcpy({{s.structure}}->{{field.struct_name}}, buffer + offset + {{field.pe_offset}}, 8);
	{{s.structure}}->{{field.struct_name}}[8] = 0;
	{% else -%}
	{{s.structure}}->{{field.struct_name}} = read_{{field.pe_type}}(buffer + offset + {{field.pe_offset}});
	{% endif -%}
	{% endfor %}

	return {{s.common_size}};
}
