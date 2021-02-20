#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "constants.h"

#define CHECK_BIT(var,val) ((var) & (val))

typedef struct struct_field {
	size_t offset;
	size_t size;
	const char* name;
	union {
		uint8_t c_val;
		uint16_t s_val;
		uint32_t i_val;
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

const struct_field_t pe_optional_headers_standard_fields[] = {
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

const struct_field_t peplus_optional_headers_standard_fields[] = {
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

typedef struct pefile {
	uint8_t* stub;
	struct_field_t coff_header[8];
	uint16_t magic;
	struct_field_t *optional_headers_standard;
} pefile_t;

uint8_t read_char(const uint8_t* buffer) {
	return *buffer;
}

uint16_t read_short(const uint8_t* buffer) {
	return *(uint16_t*)buffer;
}

uint32_t read_int(const uint8_t* buffer) {
	return *(uint32_t*)buffer;
}

struct_field_t* get_field(const char* name, struct_field_t* header) {
	struct_field_t* field = header;

	while (field->size) {
		if (strcmp(field->name, name) == 0) {
			return field;
		}
		++field;
	}

	fprintf(stderr, "Field %s not found\n", name);
	return NULL;
}

uint8_t get_field_char(const char* name, struct_field_t* header) {
	struct_field_t* field = get_field(name, header);
	if (field) {
		return field->value.c_val;
	}
	return 0;
}

uint16_t get_field_short(const char* name, struct_field_t* header) {
	struct_field_t* field = get_field(name, header);
	if (field) {
		return field->value.s_val;
	}
	return 0;
}

uint32_t get_field_int(const char* name, struct_field_t* header) {
	struct_field_t* field = get_field(name, header);
	if (field) {
		return field->value.i_val;
	}
	return 0;
}

void parse_header(const uint8_t* buffer, const struct_field_t* header, struct_field_t* dest) {
	const struct_field_t* field = header;
	uint32_t i = 0;

	while (field->size) {
		dest[i].offset = field->offset;
		dest[i].size = field->size;
		dest[i].name = field->name;

		switch (field->size) {
			case 1:
				dest[i].value.c_val = read_char(buffer + field->offset);
				break;
			case 2:
				dest[i].value.s_val = read_short(buffer + field->offset);
				break;
			case 4:
				dest[i].value.i_val = read_int(buffer + field->offset);
				break;
		}
		++field;
		++i;
	}

	dest[i].size = 0;
}

void print_coff_header(struct_field_t* header) {
	const char* machine_type = NULL;
	uint16_t machine = get_field_short("Machine", header);

	machine_type_map_entry_t* m_map = machine_type_map;
	while (m_map->machine) {
		if (m_map->type == machine) {
			machine_type = m_map->machine;
		}
		++m_map;
	}

	if (! machine_type) {
		fprintf(stderr, "Invalid machine type?");
		return;
	}

	printf("COFF header:\n");
	printf("Machine: %s\n", machine_type);
	printf("NumberOfSections: %i\n", get_field_short("NumberOfSections", header));
	printf("TimeDateStamp: %i\n", get_field_int("TimeDateStamp", header));
	printf("PointerToSymbolTable: %i\n", get_field_int("PointerToSymbolTable", header));
	printf("NumberOfSymbols: %i\n", get_field_int("NumberOfSymbols", header));
	printf("SizeOfOptionalHeader: %i\n", get_field_short("SizeOfOptionalHeader", header));
	printf("Characteristics: ");

	uint16_t characteristics = get_field_short("Characteristics", header);

	characteristics_map_entry_t* c_map = characteristics_map;
	while (c_map->characteristic) {
		if (CHECK_BIT(characteristics, c_map->type)) {
			printf("%s ", c_map->characteristic);
		}
		++c_map;
	}
	printf("\n");
	printf("\n");
}

void print_optional_headers_standard(struct_field_t* header) {
	uint16_t magic = get_field_short("Magic", header);

	printf("Optional headers:\n");
	printf("Magic: ");
	if (magic == PE32_MAGIC) printf("PE\n");
	if (magic == PE32PLUS_MAGIC) printf("PE+\n");
	printf("MajorLinkerVersion: %i\n", get_field_char("MajorLinkerVersion", header));
	printf("MinorLinkerVersion: %i\n", get_field_char("MinorLinkerVersion", header));
	printf("SizeOfCode: %i\n", get_field_int("SizeOfCode", header));
	printf("SizeOfInitializedData: %i\n", get_field_int("SizeOfInitializedData", header));
	printf("SizeOfUninitializedData: %i\n", get_field_int("SizeOfUninitializedData", header));
	printf("AddressOfEntryPoint: %i\n", get_field_int("AddressOfEntryPoint", header));
	printf("BaseOfCode: %i\n", get_field_int("BaseOfCode", header));

	if (magic == PE32_MAGIC) printf("BaseOfData: %i\n", get_field_int("BaseOfData", header));
	printf("\n");
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

int main(int argc, char* argv[]) {
	if (! argc) return 1;

	pefile_t pe;
	uint8_t* file;
	size_t size;
	uint32_t pe_header_offset;

	if (read_pe_file(argv[1], &file, &size, &pe_header_offset)) {
		return 1;
	}

	uint32_t coff_header_offset = pe_header_offset + 4;

	if (size < coff_header_offset + COFF_HEADER_SIZE) {
		fprintf(stderr, "File size too small\n");
		return 1;
	}
	
	parse_header(file + coff_header_offset, coff_header_fields, pe.coff_header);
	print_coff_header(pe.coff_header);

	uint32_t pe_optional_headers_offset = coff_header_offset + COFF_HEADER_SIZE;
	uint16_t pe_optional_headers_size = get_field_short("SizeOfOptionalHeader", pe.coff_header);

	if (! pe_optional_headers_size) {
		fprintf(stderr, "No optional headers\n");
		return 1;
	}

	if (size < pe_optional_headers_offset + pe_optional_headers_size) {
		fprintf(stderr, "File size too small\n");
		return 1;
	}
	pe.magic = read_short(file + pe_optional_headers_offset);

	if (pe.magic == PE32_MAGIC) {
		pe.optional_headers_standard = malloc(sizeof(struct_field_t) * 10);
		parse_header(file + pe_optional_headers_offset, pe_optional_headers_standard_fields, pe.optional_headers_standard);
	} else if (pe.magic == PE32PLUS_MAGIC) {
		pe.optional_headers_standard = malloc(sizeof(struct_field_t) * 9);
		parse_header(file + pe_optional_headers_offset, peplus_optional_headers_standard_fields, pe.optional_headers_standard);
	} else {
		fprintf(stderr, "Do not know how to handle this type of PE\n");
		return 1;
	}

	print_optional_headers_standard(pe.optional_headers_standard);

/*
	pe.stub = malloc(pe_header_offset);
	if (! pe.stub) {
		fprintf(stderr, "Failed to allocate memory\n");
		return 1;
	}
	memcpy(pe.stub, file, pe_header_offset);	
*/

	free(file);
	free(pe.optional_headers_standard);
	//free(pe.stub);
}
