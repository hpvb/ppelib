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

#include <stdio.h>
#include <string.h>

#include <ppelib/ppelib.h>
#include <ppelib/ppelib-low-level.h>

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Usage: %s <filename>\n", argv[0]);
		return 1;
	}

	int retval = 0;

	ppelib_handle *pe = ppelib_create_from_file(argv[1]);
	if (ppelib_error()) {
		printf("PElib-error: %s\n", ppelib_error());
		return (1);
	}

	ppelib_header_t *header = ppelib_get_header(pe);
	ppelib_print_pe_header(header);

	ppelib_free_header(header);
	ppelib_destroy(pe);

	return retval;
}
