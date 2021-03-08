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

#ifndef PPELIB_{{s.structure|upper}}_H_
#define PPELIB_{{s.structure|upper}}_H_

#include <inttypes.h>
#include <stdio.h>
#include <stddef.h>

typedef struct ppelib_{{s.structure}}_s ppelib_{{s.structure}};

{% for field in s.fields %}
{% if field.getset_type == "string_name" -%}
const char* ppelib_{{s.structure}}_get_{{field.struct_name}}(const ppelib_{{s.structure}}* {{s.structure}});
void ppelib_{{s.structure}}_set_{{field.struct_name}}(ppelib_{{s.structure}}* {{s.structure}}, const char value[9]);
{% else -%}
{{field.getset_type}} ppelib_{{s.structure}}_get_{{field.struct_name}}(const ppelib_{{s.structure}}* {{s.structure}});
{% if field.set -%}
void ppelib_{{s.structure}}_set_{{field.struct_name}}(ppelib_{{s.structure}}* {{s.structure}}, {{field.getset_type}} value);
{% endif -%}
{% if field.format and field.format.enum -%}
const char* ppelib_{{s.structure}}_get_{{field.struct_name}}_string(const ppelib_{{s.structure}}* {{s.structure}});
{% endif -%}
{% endif -%}
{% endfor %}

uint8_t ppelib_{{s.structure}}_is_null(const ppelib_{{s.structure}}* {{s.structure}});
void ppelib_{{s.structure}}_fprint(FILE* stream, const ppelib_{{s.structure}}* {{s.structure}});
void ppelib_{{s.structure}}_print(const ppelib_{{s.structure}}* {{s.structure}});

#endif /* PPELIB_{{s.structure|upper}}_H_  */
