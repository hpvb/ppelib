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
	int retval = 0;

	printf("Copying %s to %s\n", argv[1], argv[2]);

	ppelib_handle *pe = ppelib_create_from_file(argv[1]);
	if (ppelib_error()) {
		printf("PElib-error infile: %s\n", ppelib_error());
		retval = 1;
		goto out;
	}

	// Get original header
	ppelib_header_t *header = ppelib_get_header(pe);

	// Recalculate headers
	ppelib_recalculate(pe);

	// Get new headers
	ppelib_header_t *header1 = ppelib_get_header(pe);

	// Fix up unstable header fields
	header1->base_of_data = header->base_of_data;
	header1->size_of_code = header->size_of_code;
	header1->size_of_uninitialized_data = header->size_of_uninitialized_data;
	header1->size_of_initialized_data = header->size_of_initialized_data;

	ppelib_set_header(pe, header1);
	if (ppelib_error()) {
		printf("PElib-error set_header: %s\n", ppelib_error());
	}

	ppelib_free_header(header);
	ppelib_free_header(header1);

	ppelib_write_to_file(pe, argv[2]);
	if (ppelib_error()) {
		printf("PElib-error outfile: %s\n", ppelib_error());
		retval = 1;
		goto out;
	}

	out:
	ppelib_destroy(pe);

	return retval;
}
