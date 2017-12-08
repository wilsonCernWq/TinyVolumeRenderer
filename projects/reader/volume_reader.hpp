#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>

#define ERROR_TYPE 0
#define UCHAR    1
#define CHAR     2
#define UINT8    3
#define UINT16   4
#define UINT32   5
#define UINT64   6
#define INT8     7
#define INT16    8
#define INT32    9
#define INT64    10
#define FLOAT16  11
#define FLOAT32  12
#define DOUBLE64 13

bool ReadVolume
(const char* fname, int& data_type, int& data_size,
 int& data_X, int& data_Y, int& data_Z, void*& data_ptr);

size_t SizeOf(int data_type);

template<typename T>
T ReadAs(void* data, int i, int data_type){
  switch (data_type) {
    case (UCHAR):
      return ((unsigned char*)data)[i];
    case (CHAR):
      return ((char*)data)[i];
    case (UINT8):
      return ((uint8_t*)data)[i];
    case (UINT16):
      return ((uint16_t*)data)[i];
    case (UINT32):
      return ((uint32_t*)data)[i];
    case (UINT64):
      return ((uint64_t*)data)[i];
    case (INT8):
      return ((int8_t*)data)[i];
    case (INT16):
      return ((int16_t*)data)[i];
    case (INT32):
      return ((int32_t*)data)[i];
    case (FLOAT32):
      return ((float*)data)[i];
    case (DOUBLE64):
      return ((double*)data)[i];
    default:
      fprintf(stderr, "Error: Unrecognized type %i", data_type);
      return 0;
  }
}
