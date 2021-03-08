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

#include <ppelib/ppelib-low-level.h>
#include <ppelib/ppelib.h>

int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("Usage: %s <infile> <outfile>\n", argv[0]);
		return 1;
	}

	int retval = 0;

	printf("Removing VLV signature from %s, copying to %s\n", argv[1], argv[2]);

	ppelib_handle *pe = ppelib_create_from_file(argv[1]);
	if (ppelib_error()) {
		printf("PElib-error infile: %s\n", ppelib_error());
		return (1);
	}

	ppelib_dos_header *dos_header = ppelib_dos_header_get(pe);
	if (!ppelib_dos_header_has_vlv_signature(dos_header)) {
		printf("File %s has no signature\n", argv[1]);
		retval = 1;
		goto out;
	}

	ppelib_dos_header_delete_vlv_signature(dos_header);
	if (ppelib_error()) {
		printf("PElib-error ppelib_dos_header_delete_vlv_signature: %s\n", ppelib_error());
	}

	ppelib_write_to_file(pe, argv[2]);
	if (ppelib_error()) {
		printf("PElib-error outfile: %s\n", ppelib_error());
		return (1);
	}

out:
	ppelib_destroy(pe);

	return retval;
}
