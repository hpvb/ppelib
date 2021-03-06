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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "main.h"
#include "platform.h"
#include "ppe_error.h"

#include "vlv_signature_private.h"

#include "ppelib_internal.h"

EXPORT_SYM size_t ppelib_vlv_signature_get_signature_size(const vlv_signature_t* vlv_signature) {
	ppelib_reset_error();

	if (vlv_signature->signature) {
		return 128;
	}

	return 0;
}

EXPORT_SYM const uint8_t* ppelib_vlv_signature_get_signature(const vlv_signature_t* vlv_signature) {
	ppelib_reset_error();

	return vlv_signature->signature;
}

size_t find_vlv_signature(uint8_t *buffer, size_t size) {
	if (size < 4) {
		goto out;
	}

	for (size_t i = 0; i < size - 3; ++i) {
		uint32_t header = read_uint32_t(buffer + i);
		if (header == VLV_SIGNATURE) {
			return i;
		}
	}

	out: return size + 1;
}

uint8_t parse_vlv_signature(uint8_t *buffer, size_t size, vlv_signature_t *vlv_signature) {
	if (size > 128 + 16) {
		size_t vlv_offset = find_vlv_signature(buffer, size);

		if (vlv_offset > size) {
			return 1;
		}

		if (vlv_offset + 128 + 16 > size) {
			return 1;
		}

		ppelib_vlv_signature_deserialize(buffer, size, vlv_offset, vlv_signature);
		if (ppelib_error_peek()) {
			// VLV could just be part of a dos header
			ppelib_reset_error();
			return 1;
		}

		vlv_signature->signature = malloc(128);
		if (!vlv_signature->signature) {
			return 1;
		}

		memcpy(vlv_signature->signature, buffer + vlv_offset + 16, 128);
	}

	return 0;
}
