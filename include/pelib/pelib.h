#ifndef INCLUDE_PELIB_H_
#define INCLUDE_PELIB_H_

#include <stddef.h>
#include <stdint.h>

#include <pelib/pelib_constants.h>
#include <pelib/pelib-header.h>

typedef void pelib_file;

const char* pelib_error();

pelib_file* pelib_create();
void pelib_destroy(pelib_file* file);

pelib_file* pelib_create_from_buffer(uint8_t* buffer, size_t size);
pelib_file* pelib_create_from_file(const char* filename);

size_t pelib_write_to_buffer(pelib_file* file, uint8_t* buffer, size_t size);
size_t pelib_write_to_file(pelib_file* file, const char* filename);

/* Warning calling this manually with bogus headers will crash */
void pelib_recalculate(pelib_file* file);

pelib_header_t* pelib_get_header(pelib_file* file);
void pelib_free_header(pelib_header_t* header);
void pelib_set_header(pelib_file* file, pelib_header_t* header);

#endif /* INCLUDE_PELIB_H_ */
