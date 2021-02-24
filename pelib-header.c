#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "constants.h"
#include "utils.h"
#include "pelib-header.h"



/* serialize a pelib_header_t* back to the on-disk format */
/* When buffer is NULL only report how much we would write */
size_t serialize_pe_header(const pelib_header_t* header, uint8_t* buffer, size_t offset) {
  if (! buffer) {
    goto end;
  }

  if (header->magic != PE32_MAGIC && header->magic != PE32PLUS_MAGIC) {
    goto end;
  }

  uint8_t* buf = buffer + offset;
  uint8_t* directories;

  write_uint16_t(buf + 0, header->machine);
  write_uint16_t(buf + 2, header->number_of_sections);
  write_uint32_t(buf + 4, header->time_date_stamp);
  write_uint32_t(buf + 8, header->pointer_to_symbol_table);
  write_uint32_t(buf + 12, header->number_of_symbols);
  write_uint16_t(buf + 16, header->size_of_optional_header);
  write_uint16_t(buf + 18, header->characteristics);
  write_uint16_t(buf + 20, header->magic);
  write_uint8_t(buf + 22, header->major_linker_version);
  write_uint8_t(buf + 23, header->minor_linker_version);
  write_uint32_t(buf + 24, header->size_of_code);
  write_uint32_t(buf + 28, header->size_of_initialized_data);
  write_uint32_t(buf + 32, header->size_of_uninitialized_data);
  write_uint32_t(buf + 36, header->address_of_entry_point);
  write_uint32_t(buf + 40, header->base_of_code);

  if (header->magic == PE32_MAGIC) {
    write_uint32_t(buf + 44, header->base_of_data);
    write_uint32_t(buf + 48, header->image_base);
    write_uint32_t(buf + 52, header->section_alignment);
    write_uint32_t(buf + 56, header->file_alignment);
    write_uint16_t(buf + 60, header->major_operating_system_version);
    write_uint16_t(buf + 62, header->minor_operating_system_version);
    write_uint16_t(buf + 64, header->major_image_version);
    write_uint16_t(buf + 66, header->minor_image_version);
    write_uint16_t(buf + 68, header->major_subsystem_version);
    write_uint16_t(buf + 70, header->minor_subsystem_version);
    write_uint32_t(buf + 72, header->win32_version_value);
    write_uint32_t(buf + 76, header->size_of_image);
    write_uint32_t(buf + 80, header->size_of_headers);
    write_uint32_t(buf + 84, header->checksum);
    write_uint16_t(buf + 88, header->subsystem);
    write_uint16_t(buf + 90, header->dll_characteristics);
    write_uint32_t(buf + 92, header->size_of_stack_reserve);
    write_uint32_t(buf + 96, header->size_of_stack_commit);
    write_uint32_t(buf + 100, header->size_of_heap_reserve);
    write_uint32_t(buf + 104, header->size_of_heap_commit);
    write_uint32_t(buf + 108, header->loader_flags);
    write_uint32_t(buf + 112, header->number_of_rva_and_sizes);

    directories = buf + 116;
  }

  if (header->magic == PE32PLUS_MAGIC) {
    write_uint64_t(buf + 44, header->image_base);
    write_uint32_t(buf + 52, header->section_alignment);
    write_uint32_t(buf + 56, header->file_alignment);
    write_uint16_t(buf + 60, header->major_operating_system_version);
    write_uint16_t(buf + 62, header->minor_operating_system_version);
    write_uint16_t(buf + 64, header->major_image_version);
    write_uint16_t(buf + 66, header->minor_image_version);
    write_uint16_t(buf + 68, header->major_subsystem_version);
    write_uint16_t(buf + 70, header->minor_subsystem_version);
    write_uint32_t(buf + 72, header->win32_version_value);
    write_uint32_t(buf + 76, header->size_of_image);
    write_uint32_t(buf + 80, header->size_of_headers);
    write_uint32_t(buf + 84, header->checksum);
    write_uint16_t(buf + 88, header->subsystem);
    write_uint16_t(buf + 90, header->dll_characteristics);
    write_uint64_t(buf + 92, header->size_of_stack_reserve);
    write_uint64_t(buf + 100, header->size_of_stack_commit);
    write_uint64_t(buf + 108, header->size_of_heap_reserve);
    write_uint64_t(buf + 116, header->size_of_heap_commit);
    write_uint32_t(buf + 124, header->loader_flags);
    write_uint32_t(buf + 128, header->number_of_rva_and_sizes);

    directories = buf + 132;
  }

  for (uint32_t i = 0; i < header->number_of_rva_and_sizes; ++i) {
    if (i < 16) {
      write_uint32_t(directories, header->data_directories[i].virtual_address);
      write_uint32_t(directories + sizeof(uint32_t), header->data_directories[i].size);
    } else {
      write_uint32_t(directories, header->unknown_data_directories[i - 15].virtual_address);
      write_uint32_t(directories + sizeof(uint32_t), header->unknown_data_directories[i - 15].size);
    }
    directories += PE_HEADER_DATA_DIRECTORIES_SIZE;
  }

  end:
  switch (header->magic) {
    case PE32_MAGIC:
      return 116 + (header->number_of_rva_and_sizes * PE_HEADER_DATA_DIRECTORIES_SIZE);
    case PE32PLUS_MAGIC:
      return 132 + (header->number_of_rva_and_sizes * PE_HEADER_DATA_DIRECTORIES_SIZE);
    default:
      return 0;
  }
}

/* deserialize a buffer in on-disk format into a pelib_header_t */
/* Return value is the size of bytes consumed, if there is insufficient size returns 0 */
size_t deserialize_pe_header(const uint8_t* buffer, size_t offset, const size_t size, pelib_header_t* header) {
  if (size - offset < 44) {
    return 0;
  }

  const uint8_t* buf = buffer + offset;
  const uint8_t* directories;

  header->machine = read_uint16_t(buf + 0);
  header->number_of_sections = read_uint16_t(buf + 2);
  header->time_date_stamp = read_uint32_t(buf + 4);
  header->pointer_to_symbol_table = read_uint32_t(buf + 8);
  header->number_of_symbols = read_uint32_t(buf + 12);
  header->size_of_optional_header = read_uint16_t(buf + 16);
  header->characteristics = read_uint16_t(buf + 18);
  header->magic = read_uint16_t(buf + 20);
  header->major_linker_version = read_uint8_t(buf + 22);
  header->minor_linker_version = read_uint8_t(buf + 23);
  header->size_of_code = read_uint32_t(buf + 24);
  header->size_of_initialized_data = read_uint32_t(buf + 28);
  header->size_of_uninitialized_data = read_uint32_t(buf + 32);
  header->address_of_entry_point = read_uint32_t(buf + 36);
  header->base_of_code = read_uint32_t(buf + 40);

  if (header->magic != PE32_MAGIC && header->magic != PE32PLUS_MAGIC) {
    return 0;
  }

  if (header->magic == PE32_MAGIC) {
    if (size - offset < 116) {
      return 0;
    }
    header->base_of_data = read_uint32_t(buf + 44);
    header->image_base = read_uint32_t(buf + 48);
    header->section_alignment = read_uint32_t(buf + 52);
    header->file_alignment = read_uint32_t(buf + 56);
    header->major_operating_system_version = read_uint16_t(buf + 60);
    header->minor_operating_system_version = read_uint16_t(buf + 62);
    header->major_image_version = read_uint16_t(buf + 64);
    header->minor_image_version = read_uint16_t(buf + 66);
    header->major_subsystem_version = read_uint16_t(buf + 68);
    header->minor_subsystem_version = read_uint16_t(buf + 70);
    header->win32_version_value = read_uint32_t(buf + 72);
    header->size_of_image = read_uint32_t(buf + 76);
    header->size_of_headers = read_uint32_t(buf + 80);
    header->checksum = read_uint32_t(buf + 84);
    header->subsystem = read_uint16_t(buf + 88);
    header->dll_characteristics = read_uint16_t(buf + 90);
    header->size_of_stack_reserve = read_uint32_t(buf + 92);
    header->size_of_stack_commit = read_uint32_t(buf + 96);
    header->size_of_heap_reserve = read_uint32_t(buf + 100);
    header->size_of_heap_commit = read_uint32_t(buf + 104);
    header->loader_flags = read_uint32_t(buf + 108);
    header->number_of_rva_and_sizes = read_uint32_t(buf + 112);

    directories = buf + 116;
  }

  if (header->magic == PE32PLUS_MAGIC) {
    if (size - offset < 132) {
      return 0;
    }
    header->image_base = read_uint64_t(buf + 44);
    header->section_alignment = read_uint32_t(buf + 52);
    header->file_alignment = read_uint32_t(buf + 56);
    header->major_operating_system_version = read_uint16_t(buf + 60);
    header->minor_operating_system_version = read_uint16_t(buf + 62);
    header->major_image_version = read_uint16_t(buf + 64);
    header->minor_image_version = read_uint16_t(buf + 66);
    header->major_subsystem_version = read_uint16_t(buf + 68);
    header->minor_subsystem_version = read_uint16_t(buf + 70);
    header->win32_version_value = read_uint32_t(buf + 72);
    header->size_of_image = read_uint32_t(buf + 76);
    header->size_of_headers = read_uint32_t(buf + 80);
    header->checksum = read_uint32_t(buf + 84);
    header->subsystem = read_uint16_t(buf + 88);
    header->dll_characteristics = read_uint16_t(buf + 90);
    header->size_of_stack_reserve = read_uint64_t(buf + 92);
    header->size_of_stack_commit = read_uint64_t(buf + 100);
    header->size_of_heap_reserve = read_uint64_t(buf + 108);
    header->size_of_heap_commit = read_uint64_t(buf + 116);
    header->loader_flags = read_uint32_t(buf + 124);
    header->number_of_rva_and_sizes = read_uint32_t(buf + 128);

    directories = buf + 132;
  }

  if (header->number_of_rva_and_sizes > 16) {
    header->unknown_data_directories = malloc((header->number_of_rva_and_sizes - 16) * PE_HEADER_DATA_DIRECTORIES_SIZE);
  }

  for (uint32_t i = 0; i < header->number_of_rva_and_sizes; ++i) {
    if (i < 16) {
      header->data_directories[i].virtual_address = read_uint32_t(directories);
      header->data_directories[i].size = read_uint32_t(directories + sizeof(uint32_t));
    } else {
      header->unknown_data_directories[i].virtual_address = read_uint32_t(directories);
      header->unknown_data_directories[i].size = read_uint32_t(directories + sizeof(uint32_t));
    }
    directories += PE_HEADER_DATA_DIRECTORIES_SIZE;
  }

  switch (header->magic) {
    case PE32_MAGIC:
      return 116 + (header->number_of_rva_and_sizes * PE_HEADER_DATA_DIRECTORIES_SIZE);
    case PE32PLUS_MAGIC:
      return 132 + (header->number_of_rva_and_sizes * PE_HEADER_DATA_DIRECTORIES_SIZE);
    default:
      return 0;
  }
}

void print_pe_header(const pelib_header_t* header) {
  printf("Machine: %s\n", map_lookup(header->machine, machine_type_map));
  printf("NumberOfSections: %li\n", header->number_of_sections);
  printf("TimeDateStamp: %li\n", header->time_date_stamp);
  printf("PointerToSymbolTable: 0x%08X\n", header->pointer_to_symbol_table);
  printf("NumberOfSymbols: %li\n", header->number_of_symbols);
  printf("SizeOfOptionalHeader: %li\n", header->size_of_optional_header);
  printf("Characteristics (0x%08lX): ", header->characteristics);
  map_entry_t* characteristics_map_i = characteristics_map;
  while (characteristics_map_i->string) {
    if (CHECK_BIT(header->characteristics, characteristics_map_i->value)) {
      printf("%s ", characteristics_map_i->string);
    }
    ++characteristics_map_i;
  }
  printf("\n");
  printf("Magic: %s\n", map_lookup(header->magic, magic_type_map));
  printf("MajorLinkerVersion: %li\n", header->major_linker_version);
  printf("MinorLinkerVersion: %li\n", header->minor_linker_version);
  printf("SizeOfCode: %li\n", header->size_of_code);
  printf("SizeOfInitializedData: %li\n", header->size_of_initialized_data);
  printf("SizeOfUninitializedData: %li\n", header->size_of_uninitialized_data);
  printf("AddressOfEntryPoint: 0x%08X\n", header->address_of_entry_point);
  printf("BaseOfCode: 0x%08X\n", header->base_of_code);

  if (header->magic != PE32_MAGIC && header->magic != PE32PLUS_MAGIC) {
    return;
  }

  if (header->magic == PE32_MAGIC) {
    printf("BaseOfData: 0x%08X\n", header->base_of_data);
    printf("ImageBase: 0x%08X\n", header->image_base);
    printf("SectionAlignment: %li\n", header->section_alignment);
    printf("FileAlignment: %li\n", header->file_alignment);
    printf("MajorOperatingSystemVersion: %li\n", header->major_operating_system_version);
    printf("MinorOperatingSystemVersion: %li\n", header->minor_operating_system_version);
    printf("MajorImageVersion: %li\n", header->major_image_version);
    printf("MinorImageVersion: %li\n", header->minor_image_version);
    printf("MajorSubsystemVersion: %li\n", header->major_subsystem_version);
    printf("MinorSubsystemVersion: %li\n", header->minor_subsystem_version);
    printf("Win32VersionValue: %li\n", header->win32_version_value);
    printf("SizeOfImage: %li\n", header->size_of_image);
    printf("SizeOfHeaders: %li\n", header->size_of_headers);
    printf("Checksum: 0x%08X\n", header->checksum);
    printf("Subsystem: %s\n", map_lookup(header->subsystem, windows_subsystem_map));
    printf("DllCharacteristics (0x%08lX): ", header->dll_characteristics);
    map_entry_t* dll_characteristics_map_i = dll_characteristics_map;
    while (dll_characteristics_map_i->string) {
      if (CHECK_BIT(header->dll_characteristics, dll_characteristics_map_i->value)) {
        printf("%s ", dll_characteristics_map_i->string);
      }
      ++dll_characteristics_map_i;
    }
    printf("\n");
    printf("SizeOfStackReserve: %li\n", header->size_of_stack_reserve);
    printf("SizeOfStackCommit: %li\n", header->size_of_stack_commit);
    printf("SizeOfHeapReserve: %li\n", header->size_of_heap_reserve);
    printf("SizeOfHeapCommit: %li\n", header->size_of_heap_commit);
    printf("LoaderFlags: %li\n", header->loader_flags);
    printf("NumberOfRvaAndSizes: %li\n", header->number_of_rva_and_sizes);
  }
  if (header->magic == PE32PLUS_MAGIC) {
    printf("ImageBase: 0x%08X\n", header->image_base);
    printf("SectionAlignment: %li\n", header->section_alignment);
    printf("FileAlignment: %li\n", header->file_alignment);
    printf("MajorOperatingSystemVersion: %li\n", header->major_operating_system_version);
    printf("MinorOperatingSystemVersion: %li\n", header->minor_operating_system_version);
    printf("MajorImageVersion: %li\n", header->major_image_version);
    printf("MinorImageVersion: %li\n", header->minor_image_version);
    printf("MajorSubsystemVersion: %li\n", header->major_subsystem_version);
    printf("MinorSubsystemVersion: %li\n", header->minor_subsystem_version);
    printf("Win32VersionValue: %li\n", header->win32_version_value);
    printf("SizeOfImage: %li\n", header->size_of_image);
    printf("SizeOfHeaders: %li\n", header->size_of_headers);
    printf("Checksum: 0x%08X\n", header->checksum);
    printf("Subsystem: %s\n", map_lookup(header->subsystem, windows_subsystem_map));
    printf("DllCharacteristics (0x%08lX): ", header->dll_characteristics);
    map_entry_t* dll_characteristics_map_i = dll_characteristics_map;
    while (dll_characteristics_map_i->string) {
      if (CHECK_BIT(header->dll_characteristics, dll_characteristics_map_i->value)) {
        printf("%s ", dll_characteristics_map_i->string);
      }
      ++dll_characteristics_map_i;
    }
    printf("\n");
    printf("SizeOfStackReserve: %li\n", header->size_of_stack_reserve);
    printf("SizeOfStackCommit: %li\n", header->size_of_stack_commit);
    printf("SizeOfHeapReserve: %li\n", header->size_of_heap_reserve);
    printf("SizeOfHeapCommit: %li\n", header->size_of_heap_commit);
    printf("LoaderFlags: %li\n", header->loader_flags);
    printf("NumberOfRvaAndSizes: %li\n", header->number_of_rva_and_sizes);
  }
  printf("Data directories:\n");
  for (uint32_t i = 0; i < header->number_of_rva_and_sizes; ++i) {
    if (i < 16) {
      printf("%s: RVA: 0x%08lX, size: %li\n", data_directory_names[i], header->data_directories[i].virtual_address, header->data_directories[i].size);
    } else {
      printf("Unknown%i: RVA: 0x%08lX, size: %li\n", i - (16 - 1), header->unknown_data_directories[i - (16 - 1)].virtual_address, header->unknown_data_directories[i - (16 - 1)].size);
    }
  }
}