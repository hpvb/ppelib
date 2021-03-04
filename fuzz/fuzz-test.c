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

#include <stdlib.h>

#include <ppelib/ppelib-low-level.h>
#include <ppelib/ppelib.h>

int LLVMFuzzerTestOneInput(const uint8_t *buffer, size_t size) {
	ppelib_handle *pe2 = NULL;
	ppelib_handle *pe = ppelib_create_from_buffer(buffer, size);
	if (ppelib_error()) {
		printf("PPELib-Error: %s\n", ppelib_error());
		goto out;
	}
	//
	//	ppelib_recalculate(pe);
	//	ppelib_update_resource_table(pe);
	//	ppelib_recalculate(pe);

	ppelib_dos_header *dos_header = ppelib_dos_header_get(pe);

	ppelib_dos_header_delete_rich_table(dos_header);
	ppelib_dos_header_delete_vlv_signature(dos_header);

	ppelib_dos_header_set_message(dos_header,
			"A somewhat longer message than the default which should push the size of the dos stub past the default size");

	ppelib_recalculate_force(pe);

	size_t len = ppelib_write_to_buffer(pe, NULL, 0);
	if (ppelib_error()) {
		printf("PPELib-Error: %s\n", ppelib_error());
		goto out;
	}
	uint8_t *b = malloc(len);
	ppelib_write_to_buffer(pe, b, len);
	pe2 = ppelib_create_from_buffer(b, len);
	free(b);
	if (ppelib_error()) {
		printf("PPELib-Error: %s\n", ppelib_error());
		goto out;
	}
	//
out:
	ppelib_destroy(pe);
	ppelib_destroy(pe2);
	return 0; // Non-zero return values are reserved for future use.
}
