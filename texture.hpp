//===========================================================================//
//                                                                           //
// Copyright(c) 2018 Qi Wu (Wilson)                                          //
// University of California, Davis                                           //
// MIT Licensed                                                              //
//                                                                           //
//===========================================================================//

#pragma once

#include "opengl.hpp"

// Load a .BMP file using our custom loader
GLuint
loadBMP_custom(const char* imagepath);

// Load a .RAW file as 3D texture
GLuint
loadRAW_custom(const char* volumepath, int& depth);

// Load a Tfn
GLuint
loadTFN_custom();

void
updateTFN_custom(GLuint textureID, const GLubyte* palette, int width, int height);
