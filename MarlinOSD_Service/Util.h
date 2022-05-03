
#pragma once

#include <stdint.h>

#include <math.h> 


#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))


uint8_t percentToByte(uint32_t percent) { return (uint8_t)max(0, min(0xFF, (int)floor((float)percent * 2.56))); }
