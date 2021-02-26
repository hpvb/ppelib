#ifndef PELIB_MAIN_H_
#define PELIB_MAIN_H_

#include "pelib-header.h"
#include "pelib-section.h"
#include "pelib-certificate_table.h"
#include "utils.h"

typedef struct data_directory {
	pelib_section_t *section;
	uint32_t offset;
	uint32_t size;

	uint32_t orig_rva;
	uint32_t orig_size;
} data_directory_t;

typedef struct pelib_file {
	size_t pe_header_offset;
	size_t coff_header_offset;
	size_t section_offset;
	size_t start_of_sections;
	size_t end_of_sections;

	pelib_header_t header;
	pelib_section_t **sections;
	data_directory_t *data_directories;

	pelib_certificate_table_t certificate_table;

	uint8_t *stub;
	size_t trailing_data_size;
	uint8_t *trailing_data;
} pelib_file_t;

#endif /* PELIB_MAIN_H_ */
