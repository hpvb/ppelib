#include <stdint.h>

#include "utils.h"

uint8_t read_uint8_t(const uint8_t* buffer) {
        return *buffer;
}

void write_uint8_t(uint8_t* buffer, uint8_t val) {
        buffer[0] = val;
}

uint16_t read_uint16_t(const uint8_t* buffer) {
        return *(uint16_t*)buffer;
}

void write_uint16_t(uint8_t* buffer, uint16_t val) {
        ((uint16_t*)buffer)[0] = val;
}

uint32_t read_uint32_t(const uint8_t* buffer) {
        return *(uint32_t*)buffer;
}

void write_uint32_t(uint8_t* buffer, uint32_t val) {
        ((uint32_t*)buffer)[0] = val;
}

uint64_t read_uint64_t(const uint8_t* buffer) {
        return *(uint64_t*)buffer;
}

void write_uint64_t(uint8_t* buffer, uint64_t val) {
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
