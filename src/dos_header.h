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

#ifndef DOS_HEADER_H_
#define DOS_HEADER_H_

#include <inttypes.h>
#include <stdio.h>
#include <stddef.h>

#include <ppelib/ppelib-constants.h>

#include "main.h"
#include "export.h"
#include "utils.h"
#include "dos_header_private.h"

const char* default_message = "This program cannot be run in DOS mode.";
const unsigned char dos_string_end[] = { 0x0d, 0x0d, 0x0a, 0x24 }; // CR CR LF $

const unsigned char dos_stub[] = {
		 0x0e,                  // push cs
		 0x1f,                  // pop ds
		 0xba, 0x0e, 0x00,		// mov dx,0xe
		 0xb4, 0x09,            // mov ah,0x9
		 0xcd, 0x21,            // int 0x21       (puts(*(char*)0xe);
		 0xb8, 0x01, 0x4c,      // mov ax,0x4c01
		 0xcd, 0x21,            // int 0x21       (exit())
};

#endif /* DOS_HEADER_H_ */
