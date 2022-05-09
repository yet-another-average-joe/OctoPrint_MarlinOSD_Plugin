
#pragma once

#include <stdint.h>

#include <math.h> 


#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))


inline uint8_t percentToByte(uint32_t percent) { return (uint8_t)max(0, min(0xFF, (int)floor((float)percent * 2.56))); }

inline uint32_t MAKE_COLOR_ARGB8888(uint8_t a, uint8_t r, uint8_t g, uint8_t b) { return ((a << 24) | (r << 16) | (g << 8) | b); }
inline uint32_t MAKE_SOLID_COLOR_ARGB8888(uint8_t r, uint8_t g, uint8_t b) { return ((0xFF << 24) | (r << 16) | (g << 8) | b); }

