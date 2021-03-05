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
#include "dos_header.h"
#include "dos_header_private.h"

#include "ppelib_internal.h"

EXPORT_SYM const dos_header_t* ppelib_dos_header_get(const ppelib_file_t *pe) {
	ppelib_reset_error();

	return &pe->dos_header;
}

EXPORT_SYM const char* ppelib_dos_header_get_message(const dos_header_t *dos_header) {
	ppelib_reset_error();

	return dos_header->message;
}

EXPORT_SYM void ppelib_dos_header_set_message(dos_header_t *dos_header, const char *message) {
	ppelib_reset_error();

	void *oldptr = dos_header->message;
	dos_header->message = strdup(message);
	if (!dos_header->message) {
		ppelib_set_error("Failed to allocate new message");
		dos_header->message = oldptr;
		return;
	}

	update_dos_stub(dos_header);
	return;
}

// absolute string length
size_t dos_strnlen(const uint8_t *buffer, size_t len) {
	for (size_t i = 0; i < len; ++i) {
		if (buffer[i] == '$') {
			return i;
		}
	}

	return len;
}

// copy message but exclude any special characters
size_t dos_strncpy_print(char *string, const uint8_t *buffer, size_t len) {
	size_t string_len = 0;

	for (size_t i = 0; i < len; ++i) {
		if (buffer[i] == '$') {
			break;
		}

		if (isprint(buffer[i])) {
			string[string_len] = (char) buffer[i];
			string_len++;
		}
	}

	string[string_len] = 0;
	return string_len;
}

// copy char* array to '$' terminated DOS string plus linefeeds
void dos_strcpy(uint8_t *buffer, const char *string) {
	size_t string_len = strlen(string);

	memcpy(buffer, string, string_len);
	memcpy(buffer + string_len, &dos_string_end, sizeof(dos_string_end));
}

void update_dos_stub(dos_header_t *dos_header) {
	if (!dos_header->message) {
		dos_header->message = strdup(default_message);
		if (!dos_header->message) {
			return;
		}
	}

	size_t new_size = sizeof(dos_stub) + strlen(dos_header->message) + sizeof(dos_string_end);

	void *oldptr = dos_header->stub;
	dos_header->stub = realloc(dos_header->stub, new_size);
	if (!dos_header->stub) {
		dos_header->stub = oldptr;
		return;
	}

	memset(dos_header->stub, 0, new_size);
	dos_strcpy(dos_header->stub + sizeof(dos_stub), dos_header->message);
}

// Don't error for this. If this doesn't work it doesn't work.
void parse_dos_stub(dos_header_t *dos_header) {
	if (dos_header->stub_size < sizeof(dos_stub)) {
		return;
	}

	if (memcmp(dos_header->stub, dos_stub, sizeof(dos_stub)) != 0) {
		return;
	}

	size_t max_message_len = dos_header->stub_size - 0x0e;
	size_t message_len = dos_strnlen(dos_header->stub + 0xe, max_message_len);
	if (!message_len || message_len == max_message_len) {
		return;
	}

	dos_header->message = malloc(message_len + 1);
	dos_strncpy_print(dos_header->message, dos_header->stub + 0xe, message_len);
}
