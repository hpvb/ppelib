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
#ifndef PELIB_EXPORT_H_
#define PELIB_EXPORT_H_

#if defined _WIN32 || defined __CYGWIN__
	#ifdef __GNUC__
		#define EXPORT_SYM __attribute__ ((dllexport))
	#else
		#define EXPORT_SYM __declspec(dllexport)
	#endif
#else
	#if __GNUC__ >= 4
    	#define EXPORT_SYM __attribute__ ((visibility ("default")))
	#else
    	#define EXPORT_SYM
	#endif
#endif

#endif /* PELIB_EXPORT_H_ */
