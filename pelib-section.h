#ifndef PELIB_SECTION_H
#define PELIB_SECTION_H

#include <stdint.h>
#include <stddef.h>

typedef struct pelib_section {
  uint8_t name[9];
  uint32_t virtual_size;
  uint32_t virtual_address;
  uint32_t size_of_raw_data;
  uint32_t pointer_to_raw_data;
  uint32_t pointer_to_relocations;
  uint32_t pointer_to_linenumbers;
  uint16_t number_of_relocations;
  uint16_t number_of_linenumbers;
  uint32_t characteristics;
  uint8_t* contents;
} pelib_section_t;

size_t serialize_section(const pelib_section_t* section, uint8_t* buffer, size_t offset);
size_t deserialize_section(const uint8_t* buffer, size_t offset, const size_t size, pelib_section_t* section);
void print_section(const pelib_section_t* section);

#endif /* PELIB_SECTION_H */