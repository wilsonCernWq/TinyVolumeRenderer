#pragma once

#include <cstddef>
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
#define FLOAT16  11
#define FLOAT32  12
#define DOUBLE64 13

bool ReadVolume
(const char* fname, int& data_type, int& data_size,
 int& data_X, int& data_Y, int& data_Z, void*& data_ptr);

size_t SizeOf(int data_type);
