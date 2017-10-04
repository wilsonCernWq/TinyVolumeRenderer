#include "composer_object.hpp"

void ComposerObject::Init()
{
  // Compile Simple Shaders
  program = LoadProgram("vshader_slice.glsl","fshader_slice.glsl");
  ASSERT(program != 0, "Failed to create program");
  
  // Find variable locations
  texture2d_location = glGetUniformLocation(program, "tex2d");
  texture3d_location = glGetUniformLocation(program, "tex3d");
  texturetf_location = glGetUniformLocation(program, "textf");
  WARN(texture2d_location != -1, "Failed to find 'tex2d' location");
  WARN(texture3d_location != -1, "Failed to find 'tex3d' location");
  WARN(texturetf_location != -1, "Failed to find 'textf' location");
  vposition_location = glGetAttribLocation(program, "vPosition");
  ASSERT(vposition_location != -1, "Failed to find 'vPosition' location");
  vtexcoord_location = glGetAttribLocation(program, "vTexCoord");
  ASSERT(vposition_location != -1, "Failed to find 'vTexCoord' location");
  
  // Setup Vertex Array Object
  glGenVertexArrays(1, &vertex_array);
  glBindVertexArray(vertex_array);
  ASSERT(vertex_array != 0, "Failed to create vertex array object");

  // Setup Vertex Buffer Object
  glGenBuffers(2, vertex_buffer);
  ASSERT(vertex_buffer != 0, "Failed to create vertex buffer object");
}

void ComposerObject::Bind()
{
  glUseProgram(program);
  glBindVertexArray(vertex_array);
}

void ComposerObject::Compose
(const GLint texture_2d, const GLint texture_3d, const GLint texture_tf,
 const GLfloat* position_ptr, const GLfloat* texcoord_ptr, const size_t data_size)
{      
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture_2d);
  glUniform1i(texture3d_location, 0);
  
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_3D, texture_3d);
  glUniform1i(texture3d_location, 1);
  
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, texture_tf);    
  glUniform1i(texturetf_location, 2);

  glEnableVertexAttribArray(vposition_location);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * data_size, position_ptr, GL_DYNAMIC_DRAW);
  glVertexAttribPointer(vposition_location, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

  glEnableVertexAttribArray(vtexcoord_location);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * data_size, texcoord_ptr, GL_DYNAMIC_DRAW);
  glVertexAttribPointer(vtexcoord_location, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
  
  check_error_gl("in rendering");
    
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);    
  glDrawArrays(GL_TRIANGLE_FAN, 0, data_size / 3);
}
