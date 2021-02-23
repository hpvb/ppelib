#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "constants.h"

#define CHECK_BIT(var,val) ((var) & (val))
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

typedef struct struct_field {
	size_t offset;
	size_t size;
	const char* name;
	union {
		uint8_t c_val;
		uint16_t s_val;
		uint32_t i_val;
		uint64_t l_val;
		uint8_t string[9];
	} value;
} struct_field_t;

const struct_field_t coff_header_fields[] = {
	{ 0,  2, "Machine", {0}},
	{ 2,  2, "NumberOfSections", {0}},
	{ 4,  4, "TimeDateStamp", {0}},
	{ 8,  4, "PointerToSymbolTable", {0}},
	{ 12, 4, "NumberOfSymbols", {0}},
	{ 16, 2, "SizeOfOptionalHeader", {0}},
	{ 18, 2, "Characteristics", {0}},
	{ 0, 0, "", {0}}
};

const struct_field_t pe_optional_header_standard_fields[] = {
	{ 0,  2, "Magic", {0}},
	{ 2,  1, "MajorLinkerVersion", {0}},
	{ 3,  1, "MinorLinkerVersion", {0}},
	{ 4,  4, "SizeOfCode", {0}},
	{ 8,  4, "SizeOfInitializedData", {0}},
	{ 12,  4, "SizeOfUninitializedData", {0}},
	{ 16,  4, "AddressOfEntryPoint", {0}},
	{ 20,  4, "BaseOfCode", {0}},
	{ 24,  4, "BaseOfData", {0}},
	{ 0, 0, "", {0}}
};

const struct_field_t peplus_optional_header_standard_fields[] = {
	{ 0,  2, "Magic", {0}},
	{ 2,  1, "MajorLinkerVersion", {0}},
	{ 3,  1, "MinorLinkerVersion", {0}},
	{ 4,  4, "SizeOfCode", {0}},
	{ 8,  4, "SizeOfInitializedData", {0}},
	{ 12,  4, "SizeOfUninitializedData", {0}},
	{ 16,  4, "AddressOfEntryPoint", {0}},
	{ 20,  4, "BaseOfCode", {0}},
	{ 0, 0, "", {0}}
};

const struct_field_t pe_optional_header_windows_fields[] = {
	{ 0, 4, "ImageBase", {0}}, 
	{ 4, 4, "SectionAlignment", {0}}, 
	{ 8, 4, "FileAlignment", {0}}, 
	{ 12, 2, "MajorOperatingSystemVersion", {0}}, 
	{ 14, 2, "MinorOperatingSystemVersion", {0}}, 
	{ 16, 2, "MajorImageVersion", {0}}, 
	{ 18, 2, "MinorImageVersion", {0}}, 
	{ 20, 2, "MajorSubsystemVersion", {0}}, 
	{ 22, 2, "MinorSubsystemVersion", {0}}, 
	{ 24, 4, "Win32VersionValue", {0}}, 
	{ 28, 4, "SizeOfImage", {0}}, 
	{ 32, 4, "SizeOfHeaders", {0}}, 
	{ 36, 4, "CheckSum", {0}}, 
	{ 40, 2, "Subsystem", {0}}, 
	{ 42, 2, "DllCharacteristics", {0}}, 
	{ 44, 4, "SizeOfStackReserve", {0}}, 
	{ 48, 4, "SizeOfStackCommit", {0}}, 
	{ 52, 4, "SizeOfHeapReserve", {0}}, 
	{ 56, 4, "SizeOfHeapCommit", {0}}, 
	{ 60, 4, "LoaderFlags", {0}}, 
	{ 64, 4, "NumberOfRvaAndSizes", {0}},
	{ 0, 0, "", {0}}
};

const struct_field_t peplus_optional_header_windows_fields[] = {
	{ 0, 8, "ImageBase", {0}},
	{ 8, 4, "SectionAlignment", {0}},
	{ 12, 4, "FileAlignment", {0}},
	{ 16, 2, "MajorOperatingSystemVersion", {0}},
	{ 18, 2, "MinorOperatingSystemVersion", {0}},
	{ 20, 2, "MajorImageVersion", {0}},
	{ 22, 2, "MinorImageVersion", {0}},
	{ 24, 2, "MajorSubsystemVersion", {0}},
	{ 26, 2, "MinorSubsystemVersion", {0}},
	{ 28, 4, "Win32VersionValue", {0}},
	{ 32, 4, "SizeOfImage", {0}},
	{ 36, 4, "SizeOfHeaders", {0}},
	{ 40, 4, "CheckSum", {0}},
	{ 44, 2, "Subsystem", {0}},
	{ 46, 2, "DllCharacteristics", {0}},
	{ 48, 8, "SizeOfStackReserve", {0}},
	{ 56, 8, "SizeOfStackCommit", {0}},
	{ 64, 8, "SizeOfHeapReserve", {0}},
	{ 72, 8, "SizeOfHeapCommit", {0}},
	{ 80, 4, "LoaderFlags", {0}},
	{ 84, 4, "NumberOfRvaAndSizes", {0}},
	{ 0, 0, "", {0}}
};

const struct_field_t resource_directory_table_fields[] = {
	{ 0, 4, "Characteristics", {0}},
	{ 4, 4, "TimeDateStamp", {0}},
	{ 8, 2, "MajorVersion", {0}},
	{ 10, 2, "MinorVersion", {0}},
	{ 12, 2, "NumberNameEntries", {0}},
	{ 14, 2, "NumberIDEntries", {0}},
	{ 0, 0, "", {0}}
};

enum directory_entry_type {
	DIRECTORY_TYPE_NAME,
	DIRECTORY_TYPE_ID
};

typedef struct directory_entry {
	enum directory_entry_type type;
	struct_field_t fields[5];
} directory_entry_t;

const struct_field_t resource_directory_entry_fields[] = {
	{ 0, 4, "Name Offset", {0}},
	{ 0, 4, "Integer ID", {0}},
	{ 4, 4, "Data Entry Offset", {0}},
	{ 4, 4, "Subdirectory Offset", {0}},
	{ 0, 0, "", {0}}
};

typedef struct section {
	struct_field_t fields[11];
	uint8_t* contents;
} section_t;

const struct_field_t section_header_fields[] = {
	{ 0, 18, "Name", {0}}, // Bit of a hack perhaps but it's also 8 bytes wide and it's the only case.
	{ 8,  4, "VirtualSize", {0}},
	{ 12, 4, "VirtualAddress", {0}},
	{ 16, 4, "SizeOfRawData", {0}},
	{ 20, 4, "PointerToRawData", {0}},
	{ 24, 4, "PointerToRelocations", {0}},
	{ 28, 4, "PointerToLinenumbers", {0}},
	{ 32, 2, "NumberOfRelocations", {0}},
	{ 34, 2, "NumberOfLinenumbers", {0}},
	{ 36, 4, "Characteristics", {0}},
	{ 0, 0, "", {0}},
};

typedef struct directory_field {
	size_t offset;
	size_t size;
	const char* name;
	uint32_t virtual_address;
	uint32_t directory_size;
} directory_field_t;

const directory_field_t optional_header_directories[] = {
	{ 0, 8, "Export Table", 0, 0},
	{ 8, 8, "Import Table", 0, 0},
	{ 16, 8, "Resource Table", 0, 0},
	{ 24, 8, "Exception Table", 0, 0},
	{ 32, 8, "Certificate Table", 0, 0},
	{ 40, 8, "Base Relocation Table", 0, 0},
	{ 48, 8, "Debug", 0, 0},
	{ 56, 8, "Architecture", 0, 0},
	{ 64, 8, "Global Ptr", 0, 0},
	{ 72, 8, "TLS Table", 0, 0},
	{ 80, 8, "Load Config Table", 0, 0},
	{ 88, 8, "Bound Import", 0, 0},
	{ 96, 8, "IAT", 0, 0},
	{ 104, 8, "Delay Import Descriptor", 0, 0},
	{ 112, 8, "CLR Runtime Header", 0, 0},
	{ 120, 8, "Reserved", 0, 0},
	{ 0, 0, "", 0, 0},
};

typedef struct pefile {
	size_t pe_header_offset;
	size_t coff_header_offset;
	size_t optional_header_offset;
	size_t optional_header_size;
	size_t optional_header_windows_offset;
	size_t end_of_sections;

	uint8_t* stub;
	struct_field_t coff_header[8];
	uint16_t magic;
	struct_field_t *optional_header_standard;
	struct_field_t *optional_header_windows;
	directory_field_t *optional_header_directories;
	section_t *sections;

	uint8_t* trailing_data;
	size_t trailing_data_size;
} pefile_t;

uint8_t read_char(const uint8_t* buffer) {
	return *buffer;
}

void write_char(uint8_t* buffer, uint8_t val) {
	buffer[0] = val;
}

uint16_t read_short(const uint8_t* buffer) {
	return *(uint16_t*)buffer;
}

void write_short(uint8_t* buffer, uint16_t val) {
	((uint16_t*)buffer)[0] = val;
}

uint32_t read_int(const uint8_t* buffer) {
	return *(uint32_t*)buffer;
}

void write_int(uint8_t* buffer, uint32_t val) {
	((uint32_t*)buffer)[0] = val;
}

uint64_t read_long(const uint8_t* buffer) {
	return *(uint64_t*)buffer;
}

void write_long(uint8_t* buffer, uint64_t val) {
	((uint64_t*)buffer)[0] = val;
}

const char* map_lookup(uint32_t value, map_entry_t* map) {
	map_entry_t* m = map;
	while (m->string) {
		if (m->value == value) {
			return m->string;
		}
		++m;
	}

	return NULL;
}

const struct_field_t* get_field(const char* name, const struct_field_t* header) {
	const struct_field_t* field = header;

	while (field->size) {
		if (strcmp(field->name, name) == 0) {
			return field;
		}
		++field;
	}

	fprintf(stderr, "Field %s not found\n", name);
	return NULL;
}

directory_field_t* get_directory(const char* name, directory_field_t* header) {
	directory_field_t* field = header;

	while (field->size) {
		if (strcmp(field->name, name) == 0) {
			return field;
		}
		++field;
	}

	fprintf(stderr, "Directory %s not found\n", name);
	return NULL;
}

uint8_t get_field_char(const char* name, const struct_field_t* header) {
	const struct_field_t* field = get_field(name, header);
	if (field) {
		return field->value.c_val;
	}
	return 0;
}

uint16_t get_field_short(const char* name, const struct_field_t* header) {
	const struct_field_t* field = get_field(name, header);
	if (field) {
		return field->value.s_val;
	}
	return 0;
}

uint32_t get_field_int(const char* name, const struct_field_t* header) {
	const struct_field_t* field = get_field(name, header);
	if (field) {
		return field->value.i_val;
	}
	return 0;
}

uint64_t get_field_long(const char* name, const struct_field_t* header) {
	const struct_field_t* field = get_field(name, header);
	if (field) {
		return field->value.l_val;
	}
	return 0;
}

const uint8_t* get_field_string(const char* name, const struct_field_t* header) {
	const struct_field_t* field = get_field(name, header);
	if (field) {
		return field->value.string;
	}
	return 0;
}

void parse_directories(const uint8_t* buffer, uint32_t entries, const directory_field_t* directory, directory_field_t* dest) {
	const directory_field_t* field = directory;
	uint32_t i = 0;

	while (field->size && i < entries) {
		dest[i].offset = field->offset;
		dest[i].size = field->size;
		dest[i].name = field->name;
		dest[i].virtual_address = read_int(buffer + field->offset);
		dest[i].directory_size = read_int(buffer + field->offset + sizeof(uint32_t));

		++field;
		++i;
	}	

	dest[i].size = 0;
}

int write_directories(uint8_t* buffer, uint32_t entries, const directory_field_t* directory) {
	const directory_field_t* field = directory;
	uint32_t i = 0;
	size_t size = 0;

	while (field->size && i < entries) {
		size += field->size;

		if (buffer) {
			write_int(buffer + (i * 8), field->virtual_address);
			write_int(buffer + (i * 8) + 4, field->directory_size);
		}

		++field;
		++i;
	}	

	return size;
}

void parse_header(const uint8_t* buffer, const struct_field_t* header, struct_field_t* dest) {
	const struct_field_t* field = header;
	uint32_t i = 0;

	while (field->size) {
		dest[i].offset = field->offset;
		dest[i].size = field->size;
		dest[i].name = field->name;
		memset(dest[i].value.string, 0, sizeof(dest[i].value));

		if (field->size == 1) dest[i].value.c_val = read_char(buffer + field->offset);
		else if (field->size == 2) dest[i].value.s_val = read_short(buffer + field->offset);
		else if (field->size == 4) dest[i].value.i_val = read_int(buffer + field->offset);
		else if (field->size == 8) dest[i].value.l_val = read_long(buffer + field->offset);
		else if (field->size == 18) memcpy(&dest[i].value.string, buffer + field->offset, 8);
		else fprintf(stderr, "Unknown field size?\n");

		++field;
		++i;
	}

	dest[i].size = 0;
}

int write_header(uint8_t* buffer, const struct_field_t* header) {
	const struct_field_t* field = header;
	size_t size = 0;

	while (field->size) {
		if (field->size == 18)
			size += 8;
		else
			size += field->size;

		if (buffer) {
			if (field->size == 1) write_char(buffer + field->offset, field->value.c_val);
			else if (field->size == 2) write_short(buffer + field->offset, field->value.s_val);
			else if (field->size == 4) write_int(buffer + field->offset, field->value.i_val);
			else if (field->size == 8) write_long(buffer + field->offset, field->value.l_val);
			else if (field->size == 18) memcpy(buffer + field->offset, field->value.string, 8);
			else fprintf(stderr, "Unknown field size?\n");
		}

		++field;
	}

	return size;
}

size_t parse_sections(const uint8_t* buffer, size_t offset, uint16_t number, section_t* dest) {
	const uint8_t* sections = buffer + offset;
	size_t largest_offset = 0;

	for (uint32_t i = 0; i < number; ++i) {
		parse_header(sections + (i * PE_SECTION_SIZE), section_header_fields, dest[i].fields);
		printf("Parsing section: %s\n", get_field_string("Name", dest[i].fields));

		size_t SizeOfRawData = get_field_int("SizeOfRawData", dest[i].fields);
		size_t PointerToRawData = get_field_int("PointerToRawData", dest[i].fields);
		size_t offset = PointerToRawData + SizeOfRawData;

		if (offset > largest_offset) 
			largest_offset = offset;

		dest[i].contents = malloc(SizeOfRawData);
		memcpy(dest[i].contents, buffer + PointerToRawData, SizeOfRawData);
	}

	return largest_offset;
}

size_t write_sections(uint8_t* buffer, size_t offset, uint16_t number, const section_t* sections) {
	uint8_t* section_headers = buffer + offset;

	if (! buffer) {
		size_t largest_offset = 0;

		for (uint32_t i = 0; i < number; ++i) {
			size_t PointerToRawData = get_field_int("PointerToRawData", sections[i].fields);
			size_t SizeOfRawData = get_field_int("SizeOfRawData", sections[i].fields);
			size_t offset = PointerToRawData + SizeOfRawData;

			if (offset > largest_offset) 
				largest_offset = offset;
		}

		return largest_offset;
	}

	for (uint32_t i = 0; i < number; ++i) {
		const size_t PointerToRawData = get_field_int("PointerToRawData", sections[i].fields);
		const size_t SizeOfRawData = get_field_int("SizeOfRawData", sections[i].fields);

		write_header(section_headers + (i * PE_SECTION_SIZE), sections[i].fields);
		memcpy(buffer + PointerToRawData, sections[i].contents, SizeOfRawData);
	}

	return 0;
}

void print_field_name(const char* name, struct_field_t* header) {
	printf("%s: %li\n", name, get_field_long(name, header));
}

void print_field_name_hex(const char* name, struct_field_t* header) {
	printf("%s: 0x%08lX\n", name, get_field_long(name, header));
}

void print_coff_header(struct_field_t* header) {
	uint16_t machine = get_field_short("Machine", header);

	const char* machine_type = map_lookup(machine, machine_type_map);
	if (! machine_type) {
		fprintf(stderr, "Invalid machine type?");
		return;
	}

	printf("COFF header:\n");
	printf("Machine: %s\n", machine_type);
	print_field_name("NumberOfSections", header);
	print_field_name("TimeDateStamp", header);
	print_field_name("PointerToSymbolTable", header);
	print_field_name("NumberOfSymbols", header);
	print_field_name("SizeOfOptionalHeader", header);
	printf("Characteristics: ");

	uint16_t characteristics = get_field_short("Characteristics", header);

	map_entry_t* c_map = characteristics_map;
	while (c_map->string) {
		if (CHECK_BIT(characteristics, c_map->value)) {
			printf("%s ", c_map->string);
		}
		++c_map;
	}
	printf("\n");
	printf("\n");
}

void print_optional_header_standard(struct_field_t* header) {
	uint16_t magic = get_field_short("Magic", header);

	printf("Optional standard headers:\n");
	printf("Magic: ");
	if (magic == PE32_MAGIC) printf("PE\n");
	if (magic == PE32PLUS_MAGIC) printf("PE+\n");
	print_field_name("MajorLinkerVersion", header);
	print_field_name("MinorLinkerVersion", header);
	print_field_name("SizeOfCode", header);
	print_field_name("SizeOfInitializedData", header);
	print_field_name("SizeOfUninitializedData", header);
	print_field_name("AddressOfEntryPoint", header);
	print_field_name("BaseOfCode", header);

	if (magic == PE32_MAGIC) print_field_name("BaseOfData", header);
	printf("\n");
}

void print_optional_header_windows(struct_field_t* header) {
	uint16_t subsystem = get_field_short("Subsystem", header);
	uint16_t dll_characteristics = get_field_short("DllCharacteristics", header);

	printf("Optional windows headers:\n");

	print_field_name_hex("ImageBase", header);
	print_field_name("SectionAlignment", header);
	print_field_name("FileAlignment", header);
	print_field_name("MajorOperatingSystemVersion", header);
	print_field_name("MinorOperatingSystemVersion", header);
	print_field_name("MajorImageVersion", header);
	print_field_name("MinorImageVersion", header);
	print_field_name("MajorSubsystemVersion", header);
	print_field_name("MinorSubsystemVersion", header);
	print_field_name("Win32VersionValue", header);
	print_field_name("SizeOfImage", header);
	print_field_name("SizeOfHeaders", header);
	print_field_name("CheckSum", header);
	printf("Subsystem: %s\n", map_lookup(subsystem, windows_subsystem_map));
	printf("DllCharacteristics: ");
	map_entry_t* d_map = dll_characteristics_map;
	while (d_map->string) {
		if (CHECK_BIT(dll_characteristics, d_map->value)) {
			printf("%s ", d_map->string);
		}
		++d_map;
	}
	printf("\n");
	print_field_name("SizeOfStackReserve", header);
	print_field_name("SizeOfStackCommit", header);
	print_field_name("SizeOfHeapReserve", header);
	print_field_name("SizeOfHeapCommit", header);
	print_field_name("LoaderFlags", header);
	print_field_name("NumberOfRvaAndSizes", header);
	printf("\n");
}

void print_optional_header_directories(directory_field_t* header) {
	directory_field_t* d = header;

	printf("Optional data directories:\n");
	while (d->size) {
		printf("%s: RVA: 0x%08X, size: %i\n", d->name, d->virtual_address, d->directory_size);
		++d;
	}
	printf("\n");
}

void print_section_header(struct_field_t* header) {
	uint16_t section_characteristics = get_field_short("Characteristics", header);

	printf("Section:\n");
	printf("Name: %s\n", get_field_string("Name", header));
	print_field_name("VirtualSize", header);
	print_field_name_hex("VirtualAddress", header);
	print_field_name("SizeOfRawData", header);
	print_field_name_hex("PointerToRawData", header);
	print_field_name_hex("PointerToRelocations", header);
	print_field_name_hex("PointerToLinenumbers", header);
	print_field_name("NumberOfRelocations", header);
	print_field_name("NumberOfLinenumbers", header);
	printf("Characteristics: ");
	map_entry_t* map = section_flags_map;
	while (map->string) {
		if (CHECK_BIT(section_characteristics, map->value)) {
			printf("%s ", map->string);
		}
		++map;
	}
	printf("\n");
	printf("\n");
}

uint8_t* get_resource_table(pefile_t *pe) {
	directory_field_t* resource_table = get_directory("Resource Table", pe->optional_header_directories);
	if (! resource_table || resource_table->directory_size == 0) {
		return NULL;
	}

	size_t resource_rva = resource_table->virtual_address;
	size_t resource_size = resource_table->directory_size;

	uint16_t number_of_sections = get_field_short("NumberOfSections", pe->coff_header);
	section_t* section = NULL;
	size_t VirtualAddress;

	for (uint16_t i = 0; i < number_of_sections; ++i) {
		size_t SizeOfRawData = get_field_int("SizeOfRawData", pe->sections[i].fields);
		VirtualAddress = get_field_int("VirtualAddress", pe->sections[i].fields);

		if (VirtualAddress <= resource_rva && (VirtualAddress + SizeOfRawData) >= resource_rva + resource_size) {
			section = &pe->sections[i];
			break;
		}
	}

	if (! section) {
		return NULL;
	}

	return section->contents + (resource_rva - VirtualAddress);
}

void print_resources(pefile_t *pe) {
	struct_field_t fields[7];
	uint8_t* buffer = get_resource_table(pe);

	if (! buffer) {
		printf("No resource table found\n");
		return;
	}

	parse_header(buffer, resource_directory_table_fields, fields);
	printf("\n");
	print_field_name("Characteristics", fields);
	print_field_name("TimeDateStamp", fields);
	print_field_name("MajorVersion", fields);
	print_field_name("MinorVersion", fields);
	print_field_name("NumberNameEntries", fields);
	print_field_name("NumberIDEntries", fields);
	printf("\n");

	uint32_t numb_name_entries = get_field_short("NumberNameEntries", fields);
	uint32_t numb_id_entries = get_field_short("NumberIDEntries", fields);
	uint32_t numb_entries = numb_name_entries + numb_id_entries;
	uint8_t* entries_buffer = buffer + RESOURCE_DIRECTORY_TABLE_SIZE;

	directory_entry_t* entries = malloc(sizeof(directory_entry_t) * numb_entries);

	for (uint32_t i = 0; i < numb_name_entries; ++i) {
		parse_header(entries_buffer + (i * RESOURCE_DIRECTORY_ENTRY_SIZE), resource_directory_entry_fields, entries[i].fields);
		entries[i].type = DIRECTORY_TYPE_NAME;
	}

	for (uint32_t i = numb_name_entries; i < numb_entries; ++i) {
		parse_header(entries_buffer + (i * RESOURCE_DIRECTORY_ENTRY_SIZE), resource_directory_entry_fields, entries[i].fields);
		entries[i].type = DIRECTORY_TYPE_ID;
	}

	for (uint32_t i = 0; i < numb_entries; ++i) {
		if (entries[i].type == DIRECTORY_TYPE_NAME) {
			uint32_t offset = get_field_int("Name Offset", entries[i].fields) ^ 0x80000000;
			uint16_t size = *(uint16_t*)(buffer + offset);

			wchar_t* string = calloc(1, (size + 1) * sizeof(wchar_t));
			for (int i = 0; i < size; ++i) {
				string[i] = *(uint16_t*)(buffer + offset + 2 + (i * 2));
			}

			printf("0x%08X: %ls\n", offset, string);

			free(string);
		}

		if (entries[i].type == DIRECTORY_TYPE_ID) {
			const char* type = map_lookup(get_field_int("Integer ID", entries[i].fields), resource_types_map);
			printf("Type: %s\n", type);
		}

		print_field_name_hex("Name Offset", entries[i].fields);
		print_field_name_hex("Integer ID", entries[i].fields);
		print_field_name_hex("Data Entry Offset", entries[i].fields);
		print_field_name_hex("Subdirectory Offset", entries[i].fields);
		printf("\n");
	}

	free(entries);
}

int read_pe_file(const char* filename, uint8_t** file, size_t* size, uint32_t* pe_header_offset) {
	FILE *f = fopen(filename, "r");

	if (fseek(f, PE_SIGNATURE, SEEK_SET) == -1) {
		perror("Seeking to PE header offset");
		return 1;
	}

	size_t retsize;

	retsize = fread(pe_header_offset, 1, 4, f);
	if (retsize != 4) {
		fprintf(stderr, "Couldn't read PE header offset. Got %li bytes, expected 4\n", retsize);
		return 1;
	}

	if (fseek(f, *pe_header_offset, SEEK_SET) == -1) {
		perror("Seeking to PE header");
		return 1;
	}

	uint8_t signature[4];
	retsize = fread(&signature, 1, 4, f);
	if (retsize != 4) {
		fprintf(stderr, "Couldn't read PE signature. Got %li bytes, expected 4\n", retsize);
		return 1;
	}

	if (memcmp(signature, "PE\0", 4) != 0) {
		fprintf(stderr, "Not a PE file. Got 0x%X 0x%X 0x%X 0x%X, expected 0x%X 0x%X 0x%X 0x%X\n", signature[0], signature[1], signature[2], signature[3], 'P', 'E', 0, 0);
	}

	fseek(f, 0, SEEK_END);
	*size = ftell(f);
	rewind(f);
	
	*file = malloc(*size);
	if (! *file) {
		fprintf(stderr, "Unable to allocate memory\n");
		return 1;
	}

	retsize = fread(*file, 1, *size, f);
	if (retsize != *size) {
		fprintf(stderr, "Couldn't file. Got %li bytes, expected 4\n", retsize);
		return 1;
	}
	fclose(f);

	return 0;
}

int write_pe_file(const char* filename, const pefile_t* pe) {
	uint8_t* buffer = NULL;
	size_t size = 0;
	size_t write = 0;

	uint16_t directories = get_field_int("NumberOfRvaAndSizes", pe->optional_header_windows);
	uint16_t number_of_sections = get_field_short("NumberOfSections", pe->coff_header);

	// Write stub
	size += pe->pe_header_offset;
	size += 4;
	size_t coff_header_size = write_header(NULL, pe->coff_header);
	size_t optional_header_standard_size = write_header(NULL, pe->optional_header_standard);
	size_t optional_header_windows_size =  write_header(NULL, pe->optional_header_windows);
	size_t opional_header_directories_size = write_directories(NULL, directories, pe->optional_header_directories);
	size_t sections_offset = pe->optional_header_offset + pe->optional_header_size;

	size += coff_header_size + optional_header_standard_size + optional_header_windows_size + opional_header_directories_size;
	size_t sections_size = write_sections(NULL, sections_offset, number_of_sections, pe->sections);

	// Theoretically all the sections could be before the header
	if (sections_size > size) {
		size = sections_size;
	}

	size += pe->trailing_data_size;

	printf("Size of coff_header        : %li\n", coff_header_size);
	printf("Size of standard_header    : %li\n", optional_header_standard_size);
	printf("Size of windows_header     : %li\n", optional_header_windows_size);
	printf("Size of directories_header : %li\n", opional_header_directories_size);
	printf("Size of sections           : %li\n", sections_size);
	printf("Size of trailing data      : %li\n", pe->trailing_data_size);
	printf("Total size                 : %li\n", size);

	buffer = realloc(buffer, size);
	if (! buffer) {
		fprintf(stderr, "Failed to allocate\n");
		return 1;
	}
	memset(buffer, 0, size);
	//memset(buffer, 0xFF, size);

	memcpy(buffer, pe->stub, pe->pe_header_offset);
	write += pe->pe_header_offset;

	// Write PE header
	memcpy(buffer + write, "PE\0", 4);
	write += 4;

	// Write COFF header
	write_header(buffer + write, pe->coff_header);
	write += coff_header_size;

	// Write Optional header
	write_header(buffer + write, pe->optional_header_standard);
	write += optional_header_standard_size;

	// Write Windows header
	write_header(buffer + write, pe->optional_header_windows);
	write += optional_header_windows_size;

	// Write directories
	write_directories(buffer + write, directories, pe->optional_header_directories);
	write += opional_header_directories_size;

	// Write sections
	write_sections(buffer, sections_offset, number_of_sections, pe->sections);

	// Write trailing data
	memcpy(buffer + pe->end_of_sections, pe->trailing_data, pe->trailing_data_size);
	
	FILE *f = fopen(filename, "w+");
	fwrite(buffer, 1, size, f);

	fclose(f);
	free(buffer);

	return size;
}

uint32_t calculate_checksum(pefile_t *pe, uint8_t* file, size_t size) {
	const struct_field_t* checksum_field = get_field("CheckSum", pe->optional_header_windows);

	size_t checksum_offset = checksum_field->offset;
	checksum_offset += pe->optional_header_windows_offset;

	uint64_t checksum = 0;

	for (size_t i = 0; i < size; i += 4) {
		uint32_t byte = *(uint32_t*)(file + i);

		if (i == checksum_offset)
			continue;

		checksum = (checksum & 0xFFFFFFFF) + byte + (checksum >> 32);
		if (checksum > 0x100000000)
			checksum = (checksum & 0xFFFFFFFF) + (checksum >> 32);
	}

	checksum = (checksum & 0xFFFF) + (checksum >> 16);
	checksum += checksum >> 16;
	checksum = checksum & 0xFFFF;

	checksum += size;

	return checksum;
}

int main(int argc, char* argv[]) {
	if (! argc) return 1;

	pefile_t pe = {0};
	uint8_t* file;
	size_t size;
	uint32_t pe_header_offset;

	if (read_pe_file(argv[1], &file, &size, &pe_header_offset)) {
		return 1;
	}

	pe.pe_header_offset = pe_header_offset;
	pe.coff_header_offset = pe_header_offset + 4;

	if (size < pe.coff_header_offset + COFF_HEADER_SIZE) {
		fprintf(stderr, "File size too small\n");
		return 1;
	}
	
	parse_header(file + pe.coff_header_offset, coff_header_fields, pe.coff_header);
	print_coff_header(pe.coff_header);

	pe.optional_header_offset = pe.coff_header_offset + COFF_HEADER_SIZE;
	pe.optional_header_size = get_field_short("SizeOfOptionalHeader", pe.coff_header);

	if (! pe.optional_header_size) {
		fprintf(stderr, "No optional headers\n");
		return 1;
	}

	if (size < pe.optional_header_offset + pe.optional_header_size) {
		fprintf(stderr, "File size too small\n");
		return 1;
	}

	pe.magic = read_short(file + pe.optional_header_offset);

	if (pe.magic == PE32_MAGIC) {
		uint8_t* header_standard = file + pe.optional_header_offset;
		uint8_t* header_windows = header_standard + PE_OPTIONAL_HEADER_STANDARD_SIZE;
		uint8_t* header_directories = header_windows + PE_OPTIONAL_HEADER_WINDOWS_SIZE;

		pe.optional_header_windows_offset = pe.optional_header_offset + PE_OPTIONAL_HEADER_STANDARD_SIZE;

		pe.optional_header_standard = malloc(sizeof(struct_field_t) * (PE_OPTIONAL_HEADER_STANDARD_ENTRIES + 1));
		pe.optional_header_windows = malloc(sizeof(struct_field_t) * (PE_OPTIONAL_HEADER_WINDOWS_ENTRIES + 1));

		parse_header(header_standard, pe_optional_header_standard_fields, pe.optional_header_standard);
		parse_header(header_windows, pe_optional_header_windows_fields, pe.optional_header_windows);

		uint32_t optional_directories_count = get_field_int("NumberOfRvaAndSizes", pe.optional_header_windows);
		uint32_t optional_directories_space = (pe.optional_header_size - PE_OPTIONAL_HEADER_STANDARD_SIZE - PE_OPTIONAL_HEADER_WINDOWS_SIZE) / 8;

		if (optional_directories_count != optional_directories_space) {
			fprintf(stderr, "Number of data directories does not match. Header: %i, space %i\n", optional_directories_count, optional_directories_space);
			return 1;
		}

		pe.optional_header_directories = malloc(sizeof(directory_field_t) * (optional_directories_count + 1));
		parse_directories(header_directories, optional_directories_count, optional_header_directories, pe.optional_header_directories);

	} else if (pe.magic == PE32PLUS_MAGIC) {
		uint8_t* header_standard = file + pe.optional_header_offset;
		uint8_t* header_windows = header_standard + PEPLUS_OPTIONAL_HEADER_STANDARD_SIZE;
		uint8_t* header_directories = header_windows + PEPLUS_OPTIONAL_HEADER_WINDOWS_SIZE;

		pe.optional_header_windows_offset = pe.optional_header_offset + PEPLUS_OPTIONAL_HEADER_STANDARD_SIZE;

		pe.optional_header_standard = malloc(sizeof(struct_field_t) * (PEPLUS_OPTIONAL_HEADER_STANDARD_ENTRIES + 1));
		pe.optional_header_windows = malloc(sizeof(struct_field_t) * (PEPLUS_OPTIONAL_HEADER_WINDOWS_ENTRIES + 1));

		parse_header(header_standard, peplus_optional_header_standard_fields, pe.optional_header_standard);
		parse_header(header_windows, peplus_optional_header_windows_fields, pe.optional_header_windows);

		uint32_t optional_directories_count = get_field_int("NumberOfRvaAndSizes", pe.optional_header_windows);
		uint32_t optional_directories_space = (pe.optional_header_size - PEPLUS_OPTIONAL_HEADER_STANDARD_SIZE - PEPLUS_OPTIONAL_HEADER_WINDOWS_SIZE) / 8;

		if (optional_directories_count != optional_directories_space) {
			fprintf(stderr, "Number of data directories does not match. Header: %i, space %i\n", optional_directories_count, optional_directories_space);
			return 1;
		}

		pe.optional_header_directories = malloc(sizeof(directory_field_t) * (optional_directories_count + 1));
		parse_directories(header_directories, optional_directories_count, optional_header_directories, pe.optional_header_directories);

	} else {
		fprintf(stderr, "Do not know how to handle this type of PE\n");
		return 1;
	}

	print_optional_header_standard(pe.optional_header_standard);
	print_optional_header_windows(pe.optional_header_windows);
	print_optional_header_directories(pe.optional_header_directories);

	size_t sections_offset = pe.optional_header_offset + pe.optional_header_size;
        uint16_t number_of_sections = get_field_short("NumberOfSections", pe.coff_header);

	if (size < pe.optional_header_offset + pe.optional_header_size + (number_of_sections * PE_SECTION_SIZE)) {
		fprintf(stderr, "Not enough room for %i sections\n", number_of_sections);
		return 1;
	}

	pe.sections = malloc(sizeof(section_t) * number_of_sections);
	size_t end_of_sections = parse_sections(file, sections_offset, number_of_sections, pe.sections);
	pe.end_of_sections = end_of_sections;

	for (uint32_t i = 0; i < number_of_sections; ++i) {
		print_section_header(pe.sections[i].fields);
	}
	
	pe.stub = malloc(pe.pe_header_offset);
	if (! pe.stub) {
		fprintf(stderr, "Failed to allocate memory\n");
		return 1;
	}
	memcpy(pe.stub, file, pe.pe_header_offset);

	if (size > end_of_sections) {
		pe.trailing_data_size = size - end_of_sections;
		pe.trailing_data = malloc(pe.trailing_data_size);

		memcpy(pe.trailing_data, file + end_of_sections, pe.trailing_data_size);
	}

	printf("Calculated CheckSum: %i\n", calculate_checksum(&pe, file, size));

	//write_pe_file("out.exe", &pe);
	print_resources(&pe);

	for (uint16_t i = 0; i < number_of_sections; ++i) {
		free(pe.sections[i].contents);
	}

	free(file);
	free(pe.optional_header_standard);
	free(pe.optional_header_windows);
	free(pe.optional_header_directories);
	free(pe.sections);
	free(pe.stub);
	free(pe.trailing_data);
}
