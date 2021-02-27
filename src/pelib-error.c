#include <stddef.h>
#include <stdint.h>
#include <threads.h>

const char* pelib_new_error;
const char* pelib_cur_error;

const char* pelib_error() {
	pelib_cur_error = pelib_new_error;

	return pelib_cur_error;
}

void pelib_set_error(const char* error) {
	pelib_new_error = error;
}

void pelib_reset_error() {
	pelib_new_error = NULL;
}

uint32_t pelib_error_peek() {
	if (pelib_new_error)
		return 1;

	return 0;
}
