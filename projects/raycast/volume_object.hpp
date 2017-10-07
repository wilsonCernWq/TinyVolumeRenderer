#pragma once
#include "glob.hpp"

class VolumeObject {
private:
  GLuint program = 0;
  GLint texture2d_location = -1;
  GLint texture3d_location = -1;
  GLint texturetf_location = -1;
  GLint vposition_location = -1;
  GLint vtexcoord_location = -1;
  GLint samplingrate_location = -1;
  GLint mvp_location          = -1;
  GLuint vertex_array  = 0;
  GLuint vertex_buffer[2] = {0};
public: 
  void Init();
  void Draw(const GLint, const GLint, const GLint, const float sr);
};
