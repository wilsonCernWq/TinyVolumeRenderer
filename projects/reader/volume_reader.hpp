#pragma once

#include <cstddef>
#include <cstdio>
#include <cstdint>

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
#define FLOAT32  11
#define DOUBLE64 12

bool ReadVolume(const char* fname, int& data_type, int& data_size, void*& data_ptr);

inline size_t SizeOf(int data_type)
{
  switch (data_type) {
  case (UCHAR):
    return sizeof(unsigned char);
  case (CHAR):
    return sizeof(char);
  case (UINT8):
    return sizeof(uint8_t);
  case (UINT16):
    return sizeof(uint16_t);
  case (UINT32):
    return sizeof(uint32_t);
  case (UINT64):
    return sizeof(uint64_t);
  case (INT8):
    return sizeof(int8_t);
  case (INT16):
    return sizeof(int16_t);
  case (INT32):
    return sizeof(int32_t);
  case (FLOAT32):
    return sizeof(float);
  case (DOUBLE64):
    return sizeof(double);   
  default:
    fprintf(stderr, "Error: Unrecognized type %i", data_type);
    return 0;
  }
}
