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
#include <stdint.h>

#include "export.h"

_Thread_local const char* ppelib_new_error;
_Thread_local const char* ppelib_cur_error;

EXPORT_SYM const char* ppelib_error() {
	ppelib_cur_error = ppelib_new_error;

	return ppelib_cur_error;
}

void ppelib_set_error(const char* error) {
	ppelib_new_error = error;
}

void ppelib_reset_error() {
	ppelib_new_error = NULL;
}

uint32_t ppelib_error_peek() {
	if (ppelib_new_error)
		return 1;

	return 0;
}
