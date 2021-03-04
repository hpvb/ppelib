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

#ifndef PPELIB_PLATFORM_H_
#define PPELIB_PLATFORM_H_

#if defined _WIN32
#define EXPORT_SYM __declspec(dllexport)
#else
#if __GNUC__ >= 4
#define EXPORT_SYM __attribute__((visibility("default")))
#else
#define EXPORT_SYM
#endif
#endif

#ifndef thread_local
#if defined __STDC_NO_THREADS__
#define thread_local
#elif __STDC_VERSION__ >= 201112
#define thread_local _Thread_local
#elif defined __GNUC__
#define thread_local __thread
#elif defined _WIN32
#define thread_local __declspec(thread)
#else
#error "Cannot define thread_local"
#endif
#endif

#if defined _MSC_VER
#define strdup _strdup
#define gmtime_r(x, y) gmtime_s(y, x)
#endif

#endif /* PPELIB_PLATFORM_H_ */
