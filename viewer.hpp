//===========================================================================//
//                                                                           //
// Copyright(c) 2018 Qi Wu (Wilson)                                          //
// University of California, Davis                                           //
// MIT Licensed                                                              //
//                                                                           //
//===========================================================================//

#pragma once

#include "opengl.hpp"
#ifdef USE_GLM
#include <glm/fwd.hpp>
#else
#error "GLM is required here"
#endif

GLuint
LoadProgram(const char*, const char*);

void RenderGUI(GLFWwindow*, GLuint);

GLFWwindow*
InitWindow();

void
ShutdownWindow(GLFWwindow*);
