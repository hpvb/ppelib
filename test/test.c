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

#include <stdio.h>
#include <pelib/pelib.h>
#include <pelib/pelib-low-level.h>

int main(int argc, char* argv[]) {
	pelib_file* pe = pelib_create_from_file(argv[1]);
	if (pelib_error()) {
		printf("PElib-error: %s\n", pelib_error());
		return(1);
	}

	pelib_header_t* header = pelib_get_header(pe);

	pelib_recalculate(pe);

	pelib_header_t* header1 = pelib_get_header(pe);

	print_pe_header(header);
	print_pe_header(header1);

	pelib_set_header(pe, header);
	pelib_free_header(header);
	pelib_free_header(header1);

	pelib_destroy(pe);
}
