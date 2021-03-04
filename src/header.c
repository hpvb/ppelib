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
#include <string.h>
#include <stddef.h>
#include <stdlib.h>

#include "header_private.h"
#include "ppe_error.h"
#include "export.h"
#include "main.h"

EXPORT_SYM const header_t* ppelib_header_get(ppelib_file_t *pe) {
	ppelib_reset_error();

	return &pe->header;
}

EXPORT_SYM header_t* ppelib_header_copy(header_t *header) {
	ppelib_reset_error();

	header_t *retval = malloc(sizeof(header_t));
	if (!retval) {
		ppelib_set_error("Could not allocate header copy");
		return NULL;
	}

	memcpy(retval, header, sizeof(header_t));
	return retval;
}

EXPORT_SYM void ppelib_header_free_copy(header_t *header) {
	ppelib_reset_error();

	free(header);
}

EXPORT_SYM uint32_t ppelib_header_compare(header_t *header1, header_t *header2) {
	ppelib_reset_error();

	return memcmp(header1, header2, sizeof(header_t)) == 0;
}
