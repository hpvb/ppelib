#ifndef PELIB_LOW_LEVEL_H_
#define PELIB_LOW_LEVEL_H_

#include <pelib/pelib.h>

void pelib_recalculate(pelib_file* file);

pelib_header_t* pelib_get_header(pelib_file* file);
void pelib_free_header(pelib_header_t* header);
void pelib_set_header(pelib_file* file, pelib_header_t* header);

#endif /* PELIB_PELIB_LOW_LEVEL_H_ */
