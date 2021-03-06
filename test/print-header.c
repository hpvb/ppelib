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

	if (argc != 2) {
		printf("Usage: %s <filename>\n", argv[0]);
		return 1;
	}

	ppelib_handle *pe = ppelib_create_from_file(argv[1]);
	if (ppelib_error()) {
		printf("PElib-error: %s\n", ppelib_error());
		retval = 1;
		goto out;
	}

	const ppelib_dos_header *dos_header = ppelib_dos_header_get(pe);
	printf("DOS Header:\n");
	ppelib_dos_header_print(dos_header);

	if (!ppelib_dos_header_has_vlv_signature(dos_header)) {
		const char *message = ppelib_dos_header_get_message(dos_header);
		printf("DOS Message: ");
		if (message) {
			printf("%s\n", message);
		} else {
			printf("Unknown\n");
		}
	}

	if (ppelib_dos_header_has_vlv_signature(dos_header)) {
		printf("VLV signature:\n");

		const ppelib_vlv_signature *vlv_signature = ppelib_dos_header_get_vlv_signature(dos_header);
		ppelib_vlv_signature_print(vlv_signature);
		const uint8_t *signature = ppelib_vlv_signature_get_signature(vlv_signature);
		size_t signature_size = ppelib_vlv_signature_get_signature_size(vlv_signature);

		printf("Signature: ");
		for (size_t i = 0; i < signature_size; ++i) {
			printf("%02X", signature[i]);
		}
		printf("\n");
	} else {
		printf("No VLV signature\n");
	}

	const ppelib_header *header = ppelib_header_get(pe);
	printf("\nPE Header:\n");
	ppelib_header_print(header);

	printf("\nMiscellaneous\n");
	printf("Trailing data: %zi\n", ppelib_get_trailing_data_size(pe));

	printf("\nData Directories:\n");
	uint32_t directories = ppelib_header_get_number_of_rva_and_sizes(header);
	for (uint32_t i = 0; i < directories; ++i) {
		ppelib_data_directory_print(ppelib_data_directory_get(pe, i));
		printf("\n");
	}

	printf("\nSections:\n");
	uint16_t sections = ppelib_header_get_number_of_sections(header);
	for (uint16_t i = 0; i < sections; ++i) {
		ppelib_section_print(ppelib_section_get(pe, i));
		printf("\n");
	}

	out: ppelib_destroy(pe);

	return retval;
}
