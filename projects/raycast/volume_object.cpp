#include "volume_object.hpp"
#include "camera.hpp"

static const float position_buffer_data[] = {
  -1.0f,-1.0f,-1.0f,
  -1.0f,-1.0f, 1.0f,
  -1.0f, 1.0f, 1.0f,
  1.0f, 1.0f,-1.0f,
  -1.0f,-1.0f,-1.0f,
  -1.0f, 1.0f,-1.0f,
  1.0f,-1.0f, 1.0f,
  -1.0f,-1.0f,-1.0f,
  1.0f,-1.0f,-1.0f,
  1.0f, 1.0f,-1.0f,
  1.0f,-1.0f,-1.0f,
  -1.0f,-1.0f,-1.0f,
  -1.0f,-1.0f,-1.0f,
  -1.0f, 1.0f, 1.0f,
  -1.0f, 1.0f,-1.0f,
  1.0f,-1.0f, 1.0f,
  -1.0f,-1.0f, 1.0f,
  -1.0f,-1.0f,-1.0f,
  -1.0f, 1.0f, 1.0f,
  -1.0f,-1.0f, 1.0f,
  1.0f,-1.0f, 1.0f,
  1.0f, 1.0f, 1.0f,
  1.0f,-1.0f,-1.0f,
  1.0f, 1.0f,-1.0f,
  1.0f,-1.0f,-1.0f,
  1.0f, 1.0f, 1.0f,
  1.0f,-1.0f, 1.0f,
  1.0f, 1.0f, 1.0f,
  1.0f, 1.0f,-1.0f,
  -1.0f, 1.0f,-1.0f,
  1.0f, 1.0f, 1.0f,
  -1.0f, 1.0f,-1.0f,
  -1.0f, 1.0f, 1.0f,
  1.0f, 1.0f, 1.0f,
  -1.0f, 1.0f, 1.0f,
  1.0f,-1.0f, 1.0f
};

static const float texcoord_buffer_data[] = {
  0.000059f, 1.0f-0.000004f,
  0.000103f, 1.0f-0.336048f,
  0.335973f, 1.0f-0.335903f,
  1.000023f, 1.0f-0.000013f,
  0.667979f, 1.0f-0.335851f,
  0.999958f, 1.0f-0.336064f,
  0.667979f, 1.0f-0.335851f,
  0.336024f, 1.0f-0.671877f,
  0.667969f, 1.0f-0.671889f,
  1.000023f, 1.0f-0.000013f,
  0.668104f, 1.0f-0.000013f,
  0.667979f, 1.0f-0.335851f,
  0.000059f, 1.0f-0.000004f,
  0.335973f, 1.0f-0.335903f,
  0.336098f, 1.0f-0.000071f,
  0.667979f, 1.0f-0.335851f,
  0.335973f, 1.0f-0.335903f,
  0.336024f, 1.0f-0.671877f,
  1.000004f, 1.0f-0.671847f,
  0.999958f, 1.0f-0.336064f,
  0.667979f, 1.0f-0.335851f,
  0.668104f, 1.0f-0.000013f,
  0.335973f, 1.0f-0.335903f,
  0.667979f, 1.0f-0.335851f,
  0.335973f, 1.0f-0.335903f,
  0.668104f, 1.0f-0.000013f,
  0.336098f, 1.0f-0.000071f,
  0.000103f, 1.0f-0.336048f,
  0.000004f, 1.0f-0.671870f,
  0.336024f, 1.0f-0.671877f,
  0.000103f, 1.0f-0.336048f,
  0.336024f, 1.0f-0.671877f,
  0.335973f, 1.0f-0.335903f,
  0.667969f, 1.0f-0.671889f,
  1.000004f, 1.0f-0.671847f,
  0.667979f, 1.0f-0.335851f
};

void VolumeObject::Init()
{
  // Compile Simple Shaders
  program = LoadProgram("vshader_raycast.glsl","fshader_raycast.glsl");
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

  samplingrate_location = glGetUniformLocation(program, "samplingRate");
  WARN(samplingrate_location != -1, "Failed to find 'samplingRate' location");
  mvp_location = glGetUniformLocation(program, "MVP");
  WARN(mvp_location != -1, "Failed to find 'samplingRate' location");
  
  // Setup Vertex Array Object
  glGenVertexArrays(1, &vertex_array);
  glBindVertexArray(vertex_array);
  ASSERT(vertex_array != 0, "Failed to create vertex array object");

  // Setup Vertex Buffer Object
  glGenBuffers(2, vertex_buffer);
  ASSERT(vertex_buffer != 0, "Failed to create vertex buffer object");

  glEnableVertexAttribArray(vposition_location);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(position_buffer_data),
	       position_buffer_data, GL_STATIC_DRAW);
  glVertexAttribPointer(vposition_location, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

  glEnableVertexAttribArray(vtexcoord_location);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(texcoord_buffer_data),
	       texcoord_buffer_data, GL_STATIC_DRAW);
  glVertexAttribPointer(vtexcoord_location, 2, GL_FLOAT, GL_FALSE, 0, (void*) 0);

}

void VolumeObject::Draw
(const GLint texture_2d, const GLint texture_3d, const GLint texture_tf, const float sr)
{
  glUniform1f(samplingrate_location, sr);  
  glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) GetMVPMatrixPtr());
  
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

  glEnableVertexAttribArray(vtexcoord_location);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[1]);
  
  check_error_gl("in rendering");

  // We cannot clear color buffer
  // But we must clear depth
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDrawArrays(GL_TRIANGLES, 0, 36);
}
