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

#include "generated/vlv_signature_private.h"
#include "generated/dos_header_private.h"

#include "ppelib_internal.h"

const char *default_message = "This program cannot be run in DOS mode.";
const unsigned char dos_string_end[] = { 0x0d, 0x0d, 0x0a, 0x24 }; // CR CR LF $

const unsigned char dos_stub[] = { 0x0e,					// push cs
		0x1f,					// pop ds
		0xba, 0x0e, 0x00,		// mov dx,0xe
		0xb4, 0x09,				// mov ah,0x9
		0xcd, 0x21,				// int 0x21       (puts(*(char*)0xe);
		0xb8, 0x01, 0x4c,		// mov ax,0x4c01
		0xcd, 0x21,				// int 0x21       (exit())
		};

void align_pe_header_offset(dos_header_t *dos_header);

EXPORT_SYM dos_header_t* ppelib_dos_header_get(ppelib_file_t *pe) {
	ppelib_reset_error();

	return &pe->dos_header;
}

EXPORT_SYM void ppelib_dos_header_delete_vlv_signature(dos_header_t *dos_header) {
	ppelib_reset_error();

	if (!dos_header->has_vlv_signature) {
		return;
	}

	buffer_excise(&dos_header->stub, dos_header->stub_size, dos_header->vlv_signature.start,
			dos_header->vlv_signature.end);

	dos_header->has_vlv_signature = 0;
	dos_header->stub_size -= dos_header->vlv_signature.end - dos_header->vlv_signature.start;

	align_pe_header_offset(dos_header);
}

EXPORT_SYM void ppelib_dos_header_delete_rich_table(dos_header_t *dos_header) {
	ppelib_reset_error();

	printf("ppelib_dos_header_delete_rich_table\n");
	if (!dos_header->has_rich_table) {
		return;
	}

	buffer_excise(&dos_header->stub, dos_header->stub_size, dos_header->rich_table.start, dos_header->rich_table.end);

	dos_header->has_rich_table = 0;
	dos_header->stub_size -= dos_header->rich_table.end - dos_header->rich_table.start;

	align_pe_header_offset(dos_header);
}

EXPORT_SYM const char* ppelib_dos_header_get_message(const dos_header_t *dos_header) {
	ppelib_reset_error();

	return dos_header->message;
}

EXPORT_SYM void ppelib_dos_header_set_message(dos_header_t *dos_header, const char *message) {
	ppelib_reset_error();

	if (!message) {
		dos_header->message = NULL;
		return;
	}

	size_t message_len = strlen(message);
	// Slightly arbitary but we do need space for the rest of the PE file.
	if (message_len > INT16_MAX) {
		ppelib_set_error("DOS header message too long");
		return;
	}

	void *oldptr = dos_header->message;
	dos_header->message = strdup(message);
	if (!dos_header->message) {
		ppelib_set_error("Failed to allocate new message");
		dos_header->message = oldptr;
		return;
	}
	free(oldptr);

	update_dos_stub(dos_header);
	return;
}

EXPORT_SYM char ppelib_dos_header_has_vlv_signature(const dos_header_t *dos_header) {
	if (dos_header->has_vlv_signature) {
		return 1;
	}

	return 0;
}

EXPORT_SYM const vlv_signature_t* ppelib_dos_header_get_vlv_signature(const dos_header_t *dos_header) {
	ppelib_reset_error();

	if (dos_header->has_vlv_signature) {
		return &dos_header->vlv_signature;
	}

	return NULL;
}

EXPORT_SYM char ppelib_dos_header_has_rich_table(const dos_header_t *dos_header) {
	if (dos_header->has_rich_table) {
		return 1;
	}

	return 0;
}

EXPORT_SYM const rich_table_t* ppelib_dos_header_get_rich_table(const dos_header_t *dos_header) {
	ppelib_reset_error();

	if (dos_header->has_rich_table) {
		return &dos_header->rich_table;
	}

	return NULL;
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

void align_pe_header_offset(dos_header_t *dos_header) {

	if (TO_NEAREST(dos_header->stub_size, 8) != dos_header->stub_size) {
		dos_header->stub = realloc(dos_header->stub, TO_NEAREST(dos_header->stub_size, 8));
		dos_header->stub_size = TO_NEAREST(dos_header->stub_size, 8);
	}

	uint32_t new_offset = (uint32_t) dos_header->stub_size + DOS_HEADER_SIZE;
	if (new_offset != dos_header->pe_header_offset) {
		dos_header->pe_header_offset = new_offset;
		dos_header->modified = 1;
		ppelib_recalculate(dos_header->pe);
	}
}

void update_dos_stub(dos_header_t *dos_header) {
	if (!dos_header->message) {
		dos_header->message = strdup(default_message);
		if (!dos_header->message) {
			return;
		}
	}

	// This will definitely destroy the vlv signature
	dos_header->has_vlv_signature = 0;

	// TODO We don't (yet) write our own rich tables
	dos_header->has_rich_table = 0;

	size_t new_size = sizeof(dos_stub) + strlen(dos_header->message) + sizeof(dos_string_end);

	void *oldptr = dos_header->stub;
	dos_header->stub = realloc(dos_header->stub, new_size);
	if (!dos_header->stub) {
		dos_header->stub = oldptr;
		return;
	}

	dos_header->stub_size = new_size;
	memset(dos_header->stub, 0, new_size);
	dos_strcpy(dos_header->stub + sizeof(dos_stub), dos_header->message);

	align_pe_header_offset(dos_header);
}

// Don't error for this. If this doesn't work it doesn't work.
void parse_dos_stub(dos_header_t *dos_header) {
	if (parse_vlv_signature(dos_header->stub, dos_header->stub_size, &dos_header->vlv_signature) == 0) {
		dos_header->has_vlv_signature = 1;
		// VLV signatures and dos messages don't mix
		return;
	}

	if (parse_rich_table(dos_header->stub, dos_header->stub_size, &dos_header->rich_table) == 0) {
		dos_header->has_rich_table = 1;
	}

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
