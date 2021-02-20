#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "constants.h"

#define COFF_HEADER_SIZE 20
#define CHECK_BIT(var,val) ((var) & (val))

typedef struct coff_header {
	uint16_t machine;
	uint16_t nmb_sections;
	uint32_t timestamp;
	uint32_t symboltable_ptr;
	uint32_t symboltable_size;
	uint16_t optionalheader_size;
	uint16_t characteristics;
} coff_header_t;

typedef struct pefile {
	uint8_t* stub;
	coff_header_t coff_header;
} pefile_t;

uint16_t read_short(const uint8_t* buffer) {
	uint16_t retval;
	memcpy(&retval, buffer, 2);
	return retval;
}

uint32_t read_int(const uint8_t* buffer) {
	uint32_t retval;
	memcpy(&retval, buffer, 4);
	return retval;
}

coff_header_t parse_coff_header(const uint8_t* buffer) {
	coff_header_t h;

	h.machine = read_short(&buffer[0]);
	h.nmb_sections = read_short(&buffer[2]);
	h.timestamp = read_int(&buffer[4]);
	h.symboltable_ptr = read_int(&buffer[8]);
	h.symboltable_size = read_int(&buffer[12]);
	h.optionalheader_size = read_short(&buffer[16]);
	h.characteristics = read_short(&buffer[18]);

	return h;
}

void print_coff_header(coff_header_t h) {
	const char* machine_type = NULL;
	machine_type_map_entry_t* m_map = machine_type_map;
	while (m_map->machine) {
		if (m_map->type == h.machine) {
			machine_type = m_map->machine;
		}
		++m_map;
	}

	if (! machine_type) {
		fprintf(stderr, "Invalid machine type?");
		return;
	}

	printf("COFF header:\n");
	printf("Machine type: %s\n", machine_type);
	printf("Number of sections: %i\n", h.nmb_sections);
	printf("Timestamp: %i\n", h.timestamp);
	printf("Symbol table offset: %i\n", h.symboltable_ptr);
	printf("Symbol table entries: %i\n", h.symboltable_size);
	printf("Optional header size: %i\n", h.optionalheader_size);
	printf("Characteristics: ");

	characteristics_map_entry_t* c_map = characteristics_map;
	while (c_map->characteristic) {
		if (CHECK_BIT(h.characteristics, c_map->type)) {
			printf("%s ", c_map->characteristic);
		}
		++c_map;
	}
	printf("\n");
}

int main(int argc, char* argv[]) {
	if (! argc) return 1;

	pefile_t pe;

	FILE *f = fopen(argv[1], "r");

	fprintf(stderr, "Seeking to: %i\n", PE_SIGNATURE);

	if (fseek(f, PE_SIGNATURE, SEEK_SET) == -1) {
		perror("Seeking to PE header offset");
		return 1;
	}
	size_t retsize;
	uint32_t offset;

	retsize = fread(&offset, 1, 4, f);
	if (retsize != 4) {
		fprintf(stderr, "Couldn't read PE header offset. Got %li bytes, expected 4\n", retsize);
		return 1;
	}

	fprintf(stderr, "Seeking to: %i\n", offset);
	if (fseek(f, offset, SEEK_SET) == -1) {
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

	uint8_t coff_header[COFF_HEADER_SIZE];
	retsize = fread(&coff_header, 1, COFF_HEADER_SIZE, f);
	if (retsize != COFF_HEADER_SIZE) {
		fprintf(stderr, "Couldn't read COFF header. Got %li bytes, expected %i\n", retsize, COFF_HEADER_SIZE);
		return 1;
	}

	pe.coff_header = parse_coff_header(coff_header);
	print_coff_header(pe.coff_header);
	
	pe.stub = malloc(offset);
	if (! pe.stub) {
		fprintf(stderr, "Failed to allocate memory\n");
		return 1;
	}
	
	rewind(f);
	retsize = fread(pe.stub, 1, offset, f);
	if (retsize != offset) {
		fprintf(stderr, "Failed to read PE stub");
		return 1;
	}

	free(pe.stub);
	fclose(f);
}
