//===========================================================================//
//                                                                           //
// Copyright(c) 2018 Qi Wu (Wilson)                                          //
// University of California, Davis                                           //
// MIT Licensed                                                              //
//                                                                           //
//===========================================================================//

#pragma once

#include "opengl.hpp"

class VolumeObject {
private:
    GLuint program = 0;
    // geometry
    GLint camera_position = -1;
    GLint mvp_position    = -1;
    // VAO
    GLuint vertex_array       = 0;
    GLuint vertex_buffer      = 0;
    GLint  vposition_position = -1;
    // uniforms
    GLint samplingstep_location = -1;
    GLint samplingrate_location = -1;
    // texture
    GLint texture3d_location = -1;
    GLint texturetf_location = -1;

public:
    void Init();
    void Draw(const GLint, const GLint, const float, const float);
};
