/* Copyright 2021 Hein-Pieter van Braam-Stewart
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

#ifndef PELIB_LOW_LEVEL_H_
#define PELIB_LOW_LEVEL_H_

#include <pelib/pelib.h>

void pelib_recalculate(pelib_handle* file);

pelib_header_t* pelib_get_header(pelib_handle* file);
void pelib_free_header(pelib_header_t* header);
void pelib_set_header(pelib_handle* file, pelib_header_t* header);

#endif /* PELIB_PELIB_LOW_LEVEL_H_ */
