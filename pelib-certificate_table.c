#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "constants.h"
#include "utils.h"
#include "pelib-certificate_table.h"



/* serialize a pelib_certificate_table_t* back to the on-disk format */
/* When buffer is NULL only report how much we would write */
size_t serialize_certificate_table(const pelib_certificate_table_t* certificate_table, uint8_t* buffer) {
  size_t size = 0;
  size_t offset = certificate_table->offset;

  for (uint32_t i = 0; i < certificate_table->size; ++i) {
    size += certificate_table->certificates[i].length;

    if (buffer) {
      write_uint32_t(buffer + offset + 0, certificate_table->certificates[i].length);
      write_uint16_t(buffer + offset + 4, certificate_table->certificates[i].revision);
      write_uint16_t(buffer + offset + 6, certificate_table->certificates[i].certificate_type);
      memcpy(buffer + offset + 8, certificate_table->certificates[i].certificate, certificate_table->certificates[i].length - 8);
    }

    offset = TO_NEAREST(offset + certificate_table->certificates[i].length, 8);
  }

  return size + certificate_table->offset;
}

/* deserialize a buffer in on-disk format into a pelib_certificate_table_t */
/* Return value is the size of bytes consumed, if there is insufficient size returns 0 */
size_t deserialize_certificate_table(const uint8_t* buffer, pelib_header_t* header, const size_t size, pelib_certificate_table_t* certificate_table) {
  size_t table_offset = header->data_directories[DIR_CERTIFICATE_TABLE].virtual_address;
  size_t table_size = header->data_directories[DIR_CERTIFICATE_TABLE].size;

  memset(certificate_table, 0, sizeof(pelib_certificate_table_t));

  if (! table_offset || ! table_size) {
    return 0;
  }

  if (table_offset + table_size > size) {
    return 0;
  }

  size_t certificate_count = 0;
  size_t offset = table_offset;
  size_t max_offset = table_offset + table_size;

  while (offset < max_offset) {
    size_t i = certificate_count;
    certificate_count++;

    certificate_table->certificates = realloc(certificate_table->certificates, sizeof(pelib_certificate_t) * certificate_count);
    certificate_table->certificates[i].length = read_uint32_t(buffer + offset + 0);
    certificate_table->certificates[i].revision = read_uint16_t(buffer + offset + 4);
    certificate_table->certificates[i].certificate_type = read_uint16_t(buffer + offset + 6);

    if (offset + certificate_table->certificates[i].length > max_offset) {
      return 0;
    }

    certificate_table->certificates[i].certificate = malloc(certificate_table->certificates[i].length);
    memcpy(certificate_table->certificates[i].certificate, buffer + offset + 8, certificate_table->certificates[i].length - 8);

    offset = TO_NEAREST(offset + certificate_table->certificates[i].length, 8);
  }

  certificate_table->size = certificate_count;
  certificate_table->offset = table_offset;

  return max_offset;
}

void print_certificate_table(const pelib_certificate_table_t* certificate_table) {
  printf("Certificate table: \n");
  for (size_t i = 0; i < certificate_table->size; ++i) {
    printf("Certificate: %i\n", i);
    pelib_certificate_t* certificate = &certificate_table->certificates[i];
    printf("Length: %li\n", certificate->length);
    printf("Revision: %s\n", map_lookup(certificate->revision, certificate_revision_map));
    printf("CertificateType: %s\n", map_lookup(certificate->certificate_type, certificate_type_map));
  }
}