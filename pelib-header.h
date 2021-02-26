#ifndef PELIB_HEADER_H
#define PELIB_HEADER_H

#include <stdint.h>
#include <stddef.h>

#include "utils.h"

enum data_directory_type {
  DIR_EXPORT_TABLE = 0,
  DIR_IMPORT_TABLE = 1,
  DIR_RESOURCE_TABLE = 2,
  DIR_EXCEPTION_TABLE = 3,
  DIR_CERTIFICATE_TABLE = 4,
  DIR_BASE_RELOCATION_TABLE = 5,
  DIR_DEBUG = 6,
  DIR_ARCHITECTURE = 7,
  DIR_GLOBAL_PTR = 8,
  DIR_TLS_TABLE = 9,
  DIR_LOAD_CONFIG_TABLE = 10,
  DIR_BOUND_IMPORT = 11,
  DIR_IAT = 12,
  DIR_DELAY_IMPORT_DESCRIPTOR = 13,
  DIR_CLR_RUNTIME_HEADER = 14,
  DIR_RESERVED = 15,
};

static const char* data_directory_names[] = {
	"ExportTable",
	"ImportTable",
	"ResourceTable",
	"ExceptionTable",
	"CertificateTable",
	"BaseRelocationTable",
	"Debug",
	"Architecture",
	"GlobalPtr",
	"TLSTable",
	"LoadConfigTable",
	"BoundImport",
	"IAT",
	"DelayImportDescriptor",
	"CLRRuntimeHeader",
	"Reserved",
};

typedef struct pelib_data_directory {
  uint32_t virtual_address;
  uint32_t size;
} pelib_data_directory_t;

typedef struct pelib_header {
  uint16_t machine;
  uint16_t number_of_sections;
  uint32_t time_date_stamp;
  uint32_t pointer_to_symbol_table;
  uint32_t number_of_symbols;
  uint16_t size_of_optional_header;
  uint16_t characteristics;
  uint16_t magic;
  uint8_t major_linker_version;
  uint8_t minor_linker_version;
  uint32_t size_of_code;
  uint32_t size_of_initialized_data;
  uint32_t size_of_uninitialized_data;
  uint32_t address_of_entry_point;
  uint32_t base_of_code;
  uint32_t base_of_data;
  uint64_t image_base;
  uint32_t section_alignment;
  uint32_t file_alignment;
  uint16_t major_operating_system_version;
  uint16_t minor_operating_system_version;
  uint16_t major_image_version;
  uint16_t minor_image_version;
  uint16_t major_subsystem_version;
  uint16_t minor_subsystem_version;
  uint32_t win32_version_value;
  uint32_t size_of_image;
  uint32_t size_of_headers;
  uint32_t checksum;
  uint16_t subsystem;
  uint16_t dll_characteristics;
  uint64_t size_of_stack_reserve;
  uint64_t size_of_stack_commit;
  uint64_t size_of_heap_reserve;
  uint64_t size_of_heap_commit;
  uint32_t loader_flags;
  uint32_t number_of_rva_and_sizes;

  pelib_data_directory_t* data_directories;
} pelib_header_t;

size_t serialize_pe_header(const pelib_header_t* header, uint8_t* buffer, size_t offset);
size_t deserialize_pe_header(const uint8_t* buffer, size_t offset, const size_t size, pelib_header_t* header);
void print_pe_header(const pelib_header_t* header);

#endif /* PELIB_HEADER_H */