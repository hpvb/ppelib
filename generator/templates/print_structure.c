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

#include <inttypes.h>
#include <stdio.h>

#include <ppelib/ppelib-constants.h>
#include "ppe_error.h"

#include "utils.h"

#include "{{s.structure}}_private.h"

void ppelib_{{s.structure}}_printf(FILE* stream, const {{s.structure}}_t* {{s.structure}}) {
	ppelib_reset_error();

{%- for field in s.fields -%}
{%- if field.pe_only %}
	if ({{s.structure}}->magic == PE32_MAGIC) {
{%- endif %}
{%- if field.format %}
	{%- if field.format.hex %}
	{%- if field.getset_type == "uint64_t" %}
	fprintf(stream, "{{field.name}}: 0x%08" PRIx64 "\n", {{s.structure}}->{{field.struct_name}});
	{%- else %}
	fprintf(stream, "{{field.name}}: 0x%08X\n", {{s.structure}}->{{field.struct_name}});
	{%- endif %}
	{%- elif field.format.bitfield %}
	{%- if field.getset_type == "uint64_t" %}
	fprintf(stream, "{{field.name}}: (0x%08" PRIx64 ") ", {{s.structure}}->{{field.struct_name}});
	{%- else %}
	fprintf(stream, "{{field.name}}: (0x%08X) ", {{s.structure}}->{{field.struct_name}});
	{%- endif %}
	const ppelib_map_entry_t* {{field.struct_name}}_map_i = {{field.format.bitfield}};
	while ({{field.struct_name}}_map_i->string) {
		if (CHECK_BIT({{s.structure}}->{{field.struct_name}}, {{field.struct_name}}_map_i->value)) {
			fprintf(stream, "%s ", {{field.struct_name}}_map_i->string);
		}
		++{{field.struct_name}}_map_i;
	}
	fprintf(stream, "\n");
	{%- elif field.format.enum %}
	{%- if field.getset_type == "uint64_t" %}
	fprintf(stream, "{{field.name}}: (0x%08" PRIx64 ") %s\n", {{s.structure}}->{{field.struct_name}}, map_lookup({{s.structure}}->{{field.struct_name}}, {{field.format.enum}}));
	{%- else %}
	fprintf(stream, "{{field.name}}: (0x%08X) %s\n", {{s.structure}}->{{field.struct_name}}, map_lookup({{s.structure}}->{{field.struct_name}}, {{field.format.enum}}));
	{%- endif %}
	{%- endif %}
{%- else %}
	{%- if field.getset_type == "uint64_t" %}
	fprintf(stream, "{{field.name}}: %" PRIu64 "\n", {{s.structure}}->{{field.struct_name}});
	{%- else %}
	fprintf(stream, "{{field.name}}: %i\n", {{s.structure}}->{{field.struct_name}});
	{%- endif %}
{%- endif %}
{%- if field.pe_only %}
	}
{%- endif %}
{%- endfor %}
}

void ppelib_{{s.structure}}_print(const {{s.structure}}_t* {{s.structure}}) {
	ppelib_{{s.structure}}_printf(stdout, {{s.structure}});
}

