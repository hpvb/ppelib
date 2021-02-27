/* Copyright 2021 Hein-Pieter van Braam-Stewart
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

_Thread_local const char* pelib_new_error;
_Thread_local const char* pelib_cur_error;

const char* pelib_error() {
	pelib_cur_error = pelib_new_error;

	return pelib_cur_error;
}

void pelib_set_error(const char* error) {
	pelib_new_error = error;
}

void pelib_reset_error() {
	pelib_new_error = NULL;
}

uint32_t pelib_error_peek() {
	if (pelib_new_error)
		return 1;

	return 0;
}
