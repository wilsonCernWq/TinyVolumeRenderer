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

GLFWwindow*
InitWindow();

void
RenderGUI(GLFWwindow*, GLuint);

void
ShutdownWindow(GLFWwindow*);
