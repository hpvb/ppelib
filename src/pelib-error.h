#ifndef PELIB_ERROR_H_
#define PELIB_ERROR_H_

#include <stdint.h>

void pelib_set_error(const char* error);
void pelib_reset_error();

uint32_t pelib_error_peek();

#endif /* PELIB_ERROR_H_*/
