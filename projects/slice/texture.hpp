#pragma once
#ifndef TEXTURE_HPP
#define TEXTURE_HPP

// Load a .BMP file using our custom loader
GLuint loadBMP_custom(const char * imagepath);

// Load a .RAW file as 3D texture
GLuint loadRAW_custom(const char * volumepath);

#endif
