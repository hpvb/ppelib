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
#include <stddef.h>
#include <string.h>

#include "platform.h"

thread_local const char *ppelib_cur_error;
thread_local char ppelib_error_str[100];

EXPORT_SYM const char *ppelib_error() {
	return ppelib_cur_error;
}

void ppelib_set_error_func(const char *function, const char *error) {
	strncpy(ppelib_error_str, function, 99);
	strncat(ppelib_error_str, "(): ", 99);
	strncat(ppelib_error_str, error, 99);

	ppelib_cur_error = ppelib_error_str;
}

void ppelib_reset_error() {
	ppelib_cur_error = NULL;
}

uint32_t ppelib_error_peek() {
	if (ppelib_cur_error)
		return 1;

	return 0;
}
