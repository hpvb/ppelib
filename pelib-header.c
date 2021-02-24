#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "constants.h"
#include "utils.h"
#include "pelib-header.h"

/* serialize a pelib_header_t* back to the on-disk format */
/* When buffer is NULL only report how much we would write */
uint32_t serialize_pe_header(const pelib_header_t* header, uint8_t* buffer) {
  if (! buffer) {
    goto end;
  }

  if (header->magic != PE32_MAGIC && header->magic != PE32PLUS_MAGIC) {
    goto end;
  }

  uint8_t* directories;

  write_uint16_t(buffer + 0, header->machine);
  write_uint16_t(buffer + 2, header->number_of_sections);
  write_uint32_t(buffer + 4, header->time_date_stamp);
  write_uint32_t(buffer + 8, header->pointer_to_symbol_table);
  write_uint32_t(buffer + 12, header->number_of_symbols);
  write_uint16_t(buffer + 16, header->size_of_optional_header);
  write_uint16_t(buffer + 18, header->characteristics);
  write_uint16_t(buffer + 20, header->magic);
  write_uint8_t(buffer + 22, header->major_linker_version);
  write_uint8_t(buffer + 23, header->minor_linker_version);
  write_uint32_t(buffer + 24, header->size_of_code);
  write_uint32_t(buffer + 28, header->size_of_initialized_data);
  write_uint32_t(buffer + 32, header->size_of_uninitialized_data);
  write_uint32_t(buffer + 36, header->address_of_entry_point);
  write_uint32_t(buffer + 40, header->base_of_code);

  if (header->magic == PE32_MAGIC) {
    write_uint32_t(buffer + 44, header->base_of_data);
    write_uint32_t(buffer + 48, header->image_base);
    write_uint32_t(buffer + 52, header->section_alignment);
    write_uint32_t(buffer + 56, header->file_alignment);
    write_uint16_t(buffer + 60, header->major_operating_system_version);
    write_uint16_t(buffer + 62, header->minor_operating_system_version);
    write_uint16_t(buffer + 64, header->major_image_version);
    write_uint16_t(buffer + 66, header->minor_image_version);
    write_uint16_t(buffer + 68, header->major_subsystem_version);
    write_uint16_t(buffer + 70, header->minor_subsystem_version);
    write_uint32_t(buffer + 72, header->win32_version_value);
    write_uint32_t(buffer + 76, header->size_of_image);
    write_uint32_t(buffer + 80, header->size_of_headers);
    write_uint32_t(buffer + 84, header->checksum);
    write_uint16_t(buffer + 88, header->subsystem);
    write_uint16_t(buffer + 90, header->dll_characteristics);
    write_uint32_t(buffer + 92, header->size_of_stack_reserve);
    write_uint32_t(buffer + 96, header->size_of_stack_commit);
    write_uint32_t(buffer + 100, header->size_of_heap_reserve);
    write_uint32_t(buffer + 104, header->size_of_heap_commit);
    write_uint32_t(buffer + 108, header->loader_flags);
    write_uint32_t(buffer + 112, header->number_of_rva_and_sizes);

    directories = buffer + 116;
  }

  if (header->magic == PE32PLUS_MAGIC) {
    write_uint64_t(buffer + 44, header->image_base);
    write_uint32_t(buffer + 52, header->section_alignment);
    write_uint32_t(buffer + 56, header->file_alignment);
    write_uint16_t(buffer + 60, header->major_operating_system_version);
    write_uint16_t(buffer + 62, header->minor_operating_system_version);
    write_uint16_t(buffer + 64, header->major_image_version);
    write_uint16_t(buffer + 66, header->minor_image_version);
    write_uint16_t(buffer + 68, header->major_subsystem_version);
    write_uint16_t(buffer + 70, header->minor_subsystem_version);
    write_uint32_t(buffer + 72, header->win32_version_value);
    write_uint32_t(buffer + 76, header->size_of_image);
    write_uint32_t(buffer + 80, header->size_of_headers);
    write_uint32_t(buffer + 84, header->checksum);
    write_uint16_t(buffer + 88, header->subsystem);
    write_uint16_t(buffer + 90, header->dll_characteristics);
    write_uint64_t(buffer + 92, header->size_of_stack_reserve);
    write_uint64_t(buffer + 100, header->size_of_stack_commit);
    write_uint64_t(buffer + 108, header->size_of_heap_reserve);
    write_uint64_t(buffer + 116, header->size_of_heap_commit);
    write_uint32_t(buffer + 124, header->loader_flags);
    write_uint32_t(buffer + 128, header->number_of_rva_and_sizes);

    directories = buffer + 132;
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
uint32_t deserialize_pe_header(const uint8_t* buffer, const size_t size, pelib_header_t* header) {
  if (size < 44) {
    return 0;
  }

  const uint8_t* directories;

  header->machine = read_uint16_t(buffer + 0);
  header->number_of_sections = read_uint16_t(buffer + 2);
  header->time_date_stamp = read_uint32_t(buffer + 4);
  header->pointer_to_symbol_table = read_uint32_t(buffer + 8);
  header->number_of_symbols = read_uint32_t(buffer + 12);
  header->size_of_optional_header = read_uint16_t(buffer + 16);
  header->characteristics = read_uint16_t(buffer + 18);
  header->magic = read_uint16_t(buffer + 20);
  header->major_linker_version = read_uint8_t(buffer + 22);
  header->minor_linker_version = read_uint8_t(buffer + 23);
  header->size_of_code = read_uint32_t(buffer + 24);
  header->size_of_initialized_data = read_uint32_t(buffer + 28);
  header->size_of_uninitialized_data = read_uint32_t(buffer + 32);
  header->address_of_entry_point = read_uint32_t(buffer + 36);
  header->base_of_code = read_uint32_t(buffer + 40);

  if (header->magic != PE32_MAGIC && header->magic != PE32PLUS_MAGIC) {
    return 0;
  }

  if (header->magic == PE32_MAGIC) {
    if (size < 116) {
      return 0;
    }
    header->base_of_data = read_uint32_t(buffer + 44);
    header->image_base = read_uint32_t(buffer + 48);
    header->section_alignment = read_uint32_t(buffer + 52);
    header->file_alignment = read_uint32_t(buffer + 56);
    header->major_operating_system_version = read_uint16_t(buffer + 60);
    header->minor_operating_system_version = read_uint16_t(buffer + 62);
    header->major_image_version = read_uint16_t(buffer + 64);
    header->minor_image_version = read_uint16_t(buffer + 66);
    header->major_subsystem_version = read_uint16_t(buffer + 68);
    header->minor_subsystem_version = read_uint16_t(buffer + 70);
    header->win32_version_value = read_uint32_t(buffer + 72);
    header->size_of_image = read_uint32_t(buffer + 76);
    header->size_of_headers = read_uint32_t(buffer + 80);
    header->checksum = read_uint32_t(buffer + 84);
    header->subsystem = read_uint16_t(buffer + 88);
    header->dll_characteristics = read_uint16_t(buffer + 90);
    header->size_of_stack_reserve = read_uint32_t(buffer + 92);
    header->size_of_stack_commit = read_uint32_t(buffer + 96);
    header->size_of_heap_reserve = read_uint32_t(buffer + 100);
    header->size_of_heap_commit = read_uint32_t(buffer + 104);
    header->loader_flags = read_uint32_t(buffer + 108);
    header->number_of_rva_and_sizes = read_uint32_t(buffer + 112);

    directories = buffer + 116;
  }

  if (header->magic == PE32PLUS_MAGIC) {
    if (size < 132) {
      return 0;
    }
    header->image_base = read_uint64_t(buffer + 44);
    header->section_alignment = read_uint32_t(buffer + 52);
    header->file_alignment = read_uint32_t(buffer + 56);
    header->major_operating_system_version = read_uint16_t(buffer + 60);
    header->minor_operating_system_version = read_uint16_t(buffer + 62);
    header->major_image_version = read_uint16_t(buffer + 64);
    header->minor_image_version = read_uint16_t(buffer + 66);
    header->major_subsystem_version = read_uint16_t(buffer + 68);
    header->minor_subsystem_version = read_uint16_t(buffer + 70);
    header->win32_version_value = read_uint32_t(buffer + 72);
    header->size_of_image = read_uint32_t(buffer + 76);
    header->size_of_headers = read_uint32_t(buffer + 80);
    header->checksum = read_uint32_t(buffer + 84);
    header->subsystem = read_uint16_t(buffer + 88);
    header->dll_characteristics = read_uint16_t(buffer + 90);
    header->size_of_stack_reserve = read_uint64_t(buffer + 92);
    header->size_of_stack_commit = read_uint64_t(buffer + 100);
    header->size_of_heap_reserve = read_uint64_t(buffer + 108);
    header->size_of_heap_commit = read_uint64_t(buffer + 116);
    header->loader_flags = read_uint32_t(buffer + 124);
    header->number_of_rva_and_sizes = read_uint32_t(buffer + 128);

    directories = buffer + 132;
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
}

void print_pe_header(const pelib_header_t* header) {
  printf("Machine: %s\n", map_lookup(header->machine, machine_type_map));
  printf("NumberOfSections: %li\n", header->number_of_sections);
  printf("TimeDateStamp: %li\n", header->time_date_stamp);
  printf("PointerToSymbolTable: 0x%08X\n", header->pointer_to_symbol_table);
  printf("NumberOfSymbols: %li\n", header->number_of_symbols);
  printf("SizeOfOptionalHeader: %li\n", header->size_of_optional_header);
  printf("Characteristics: ");
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
    printf("DllCharacteristics: ");
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
    printf("DllCharacteristics: ");
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