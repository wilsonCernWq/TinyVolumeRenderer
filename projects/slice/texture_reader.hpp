#pragma once
#ifndef TEXTURE_READER_HPP
#define TEXTURE_READER_HPP
#include "comm.hpp"

// Load a .BMP file using our custom loader
GLuint loadBMP_custom(const char * imagepath);

// Load a .RAW file as 3D texture
GLuint loadRAW_custom(const char * volumepath);

// Load a Tfn
GLuint loadTFN_custom();
#endif
