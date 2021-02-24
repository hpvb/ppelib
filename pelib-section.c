#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "constants.h"
#include "utils.h"
#include "pelib-section.h"



/* serialize a pelib_section_t* back to the on-disk format */
/* When buffer is NULL only report how much we would write */
size_t serialize_section(const pelib_section_t* section, uint8_t* buffer, size_t offset) {
  size_t data_size = MIN(section->virtual_size, section->size_of_raw_data);

  if (! buffer) {
    goto end;
  }

  uint8_t* section_header = buffer + offset;
  memcpy(section_header + 0, section->name, 8);
  write_uint32_t(section_header + 8, section->virtual_size);
  write_uint32_t(section_header + 12, section->virtual_address);
  write_uint32_t(section_header + 16, section->size_of_raw_data);
  write_uint32_t(section_header + 20, section->pointer_to_raw_data);
  write_uint32_t(section_header + 24, section->pointer_to_relocations);
  write_uint32_t(section_header + 28, section->pointer_to_linenumbers);
  write_uint16_t(section_header + 32, section->number_of_relocations);
  write_uint16_t(section_header + 34, section->number_of_linenumbers);
  write_uint32_t(section_header + 36, section->characteristics);

  if (data_size) {
    memcpy(buffer + section->pointer_to_raw_data, section->contents, data_size);
  }

  end:
  if (section->pointer_to_raw_data > offset) {
    return section->pointer_to_raw_data + data_size;
  } else {
    return PE_SECTION_HEADER_SIZE;
  }
}

/* deserialize a buffer in on-disk format into a pelib_section_t */
/* Return value is the size of bytes consumed, if there is insufficient size returns 0 */
size_t deserialize_section(const uint8_t* buffer, size_t offset, const size_t size, pelib_section_t* section) {
  if (size - offset < PE_SECTION_HEADER_SIZE) {
    return 0;
  }

  memset(section, 0, sizeof(pelib_section_t));
  const uint8_t* section_header = buffer + offset;
  memcpy(section->name, section_header + 0, 8);
  section->virtual_size = read_uint32_t(section_header + 8);
  section->virtual_address = read_uint32_t(section_header + 12);
  section->size_of_raw_data = read_uint32_t(section_header + 16);
  section->pointer_to_raw_data = read_uint32_t(section_header + 20);
  section->pointer_to_relocations = read_uint32_t(section_header + 24);
  section->pointer_to_linenumbers = read_uint32_t(section_header + 28);
  section->number_of_relocations = read_uint16_t(section_header + 32);
  section->number_of_linenumbers = read_uint16_t(section_header + 34);
  section->characteristics = read_uint32_t(section_header + 36);

  size_t data_size = MIN(section->virtual_size, section->size_of_raw_data);

  if (section->pointer_to_raw_data + data_size > size) {
    return 0;
  }

  if (data_size) {
    section->contents = malloc(data_size);
    memcpy(section->contents, buffer + section->pointer_to_raw_data, data_size);
  }

  if (section->pointer_to_raw_data > offset) {
    return section->pointer_to_raw_data + data_size;
  } else {
    return PE_SECTION_HEADER_SIZE;
  }
}

void print_section(const pelib_section_t* section) {
  printf("Name: %s\n", section->name);
  printf("VirtualSize: %li\n", section->virtual_size);
  printf("VirtualAddress: 0x%08X\n", section->virtual_address);
  printf("SizeOfRawData: %li\n", section->size_of_raw_data);
  printf("PointerToRawData: 0x%08X\n", section->pointer_to_raw_data);
  printf("PointerToRelocations: 0x%08X\n", section->pointer_to_relocations);
  printf("PointerToLinenumbers: 0x%08X\n", section->pointer_to_linenumbers);
  printf("NumberOfRelocations: %li\n", section->number_of_relocations);
  printf("NumberOfLinenumbers: %li\n", section->number_of_linenumbers);
  printf("Characteristics (0x%08lX): ", section->characteristics);
  map_entry_t* characteristics_map_i = section_flags_map;
  while (characteristics_map_i->string) {
    if (CHECK_BIT(section->characteristics, characteristics_map_i->value)) {
      printf("%s ", characteristics_map_i->string);
    }
    ++characteristics_map_i;
  }
  printf("\n");
}