#ifndef PELIB_UTILS_H
#define PELIB_UTILS_H

#include <stdint.h>
#include <stddef.h>

#include "../include/pelib/pelib-constants.h"

#define CHECK_BIT(var,val) ((var) & (val))
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X ,Y) (((X) > (Y)) ? (X) : (Y))
#define TO_NEAREST(num, size) num + size - 1 - (num + size - 1) % size
#define SWAP(x, y) do { uint8_t swap_temp[sizeof(x)]; memcpy(swap_temp, x, sizeof(x)); x = y; memcpy(y, swap_temp, sizeof(x)); }  while (0)

uint8_t read_uint8_t(const uint8_t* buffer);
void write_uint8_t(uint8_t* buffer, uint8_t val);
uint16_t read_uint16_t(const uint8_t* buffer);
void write_uint16_t(uint8_t* buffer, uint16_t val);
uint32_t read_uint32_t(const uint8_t* buffer);
void write_uint32_t(uint8_t* buffer, uint32_t val);
uint64_t read_uint64_t(const uint8_t* buffer);
void write_uint64_t(uint8_t* buffer, uint64_t val);

const char* map_lookup(uint32_t value, const map_entry_t* map);

#endif
