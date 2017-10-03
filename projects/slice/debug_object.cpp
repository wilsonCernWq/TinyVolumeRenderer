#include "debug_object.hpp"

static float vertex_buffer_data[] = {
   1.f, 1.f,0.f,
  -1.f, 1.f,0.f,
   1.f,-1.f,0.f,
  -1.f,-1.f,0.f,
};

void DebugQuad::Init()
{
  program = LoadProgram("vshader_quad.glsl","fshader_quad.glsl");
  ASSERT(program != 0, "Failed to create program");
  
  glGenVertexArrays(1, &vertex_array);
  glBindVertexArray(vertex_array);
  
  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data),
	       vertex_buffer_data, GL_STATIC_DRAW);
  
  vposition_location = glGetAttribLocation(program, "vPosition");
  WARN(vposition_location != -1, "Failed to find 'vPosition' location");
  texture2d_location = glGetUniformLocation(program, "tex2d");  
  WARN(texture2d_location   != -1, "Failed to find 'tex' location");  
}

void DebugQuad::Draw(GLint textureID)
{
  glUseProgram(program);
  glBindVertexArray(vertex_array);
  
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textureID);    
  glUniform1i(texture2d_location, 0);
  
  glEnableVertexAttribArray(vposition_location);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glVertexAttribPointer(vposition_location, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glDisableVertexAttribArray(vposition_location);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glUseProgram(0);
}
