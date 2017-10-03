#pragma once
#include "glob.hpp"

class DebugQuad {
private:
  GLuint program = 0;
  GLuint vertex_array;
  GLuint vertex_buffer;
  GLint vposition_location = -1;
  GLint texture2d_location = -1;
public:
  DebugQuad() = default;
  void Init();
  void Draw(GLint);
};
