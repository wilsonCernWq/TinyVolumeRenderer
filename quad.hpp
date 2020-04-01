//===========================================================================//
//                                                                           //
// Copyright(c) 2018 Qi Wu (Wilson)                                          //
// University of California, Davis                                           //
// MIT Licensed                                                              //
//                                                                           //
//===========================================================================//

#pragma once

#include "viewer.hpp"

class ScreenObject {
private:
    GLuint program = 0;
    GLuint vertex_array;
    GLuint vertex_buffer;
    GLint  vposition_location = -1;
    GLint  texture2d_location = -1;

public:
    ScreenObject() = default;
    void Init();
    void Draw(GLint);
};
