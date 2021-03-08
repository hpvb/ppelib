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

#ifndef PPELIB_ERROR_H_
#define PPELIB_ERROR_H_

#include <inttypes.h>

#include "platform.h"

EXPORT_SYM const char *ppelib_error();

#define ppelib_set_error(x) ppelib_set_error_func(__FUNCTION__, x)

void ppelib_set_error_func(const char *function, const char *error);
void ppelib_reset_error();

uint32_t ppelib_error_peek();

#endif /* PPELIB_ERROR_H_*/
