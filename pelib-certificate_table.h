#ifndef PELIB_CERTIFICATE_TABLE_H
#define PELIB_CERTIFICATE_TABLE_H

#include <stdint.h>
#include <stddef.h>
#include "pelib-header.h"

typedef struct pelib_certificate {
  uint32_t length;
  uint16_t revision;
  uint16_t certificate_type;
  uint8_t* certificate;
} pelib_certificate_t;

typedef struct pelib_certificate_table {
  size_t size;
  size_t offset;
  pelib_certificate_t* certificates;
} pelib_certificate_table_t;

size_t serialize_certificate_table(const pelib_certificate_table_t* certificate_table, uint8_t* buffer);
size_t deserialize_certificate_table(const uint8_t* buffer, pelib_header_t* header, const size_t size, pelib_certificate_table_t* certificate_table);
void print_certificate_table(const pelib_certificate_table_t* certificate_table);

#endif /* PELIB_CERTIFICATE_TABLE_H */