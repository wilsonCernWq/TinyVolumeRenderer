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

void VolumeObject::Init()
{
  // Compile Simple Shaders  
  program = LoadProgram("raycast/vshader_raycast.glsl","raycast/fshader_raycast.glsl");
  ASSERT(program != 0, "Failed to create program");
  vposition_position = glGetAttribLocation(program, "vPosition");
  ASSERT(vposition_position != -1, "Failed to find 'vPosition' location");
  
  // Find variable locations
  texture3d_location = glGetUniformLocation(program, "tex3d");
  texturetf_location = glGetUniformLocation(program, "textf");
  WARN(texture3d_location != -1, "Failed to find 'tex3d' location");
  WARN(texturetf_location != -1, "Failed to find 'textf' location");
  
  samplingrate_location = glGetUniformLocation(program, "samplingRate");
  WARN(samplingrate_location != -1, "Failed to find 'samplingRate' location");

  samplingstep_location = glGetUniformLocation(program, "samplingStep");
  WARN(samplingstep_location != -1, "Failed to find 'samplingStep' location");

  camera_position = glGetUniformLocation(program, "camera");
  WARN(camera_position != -1, "Failed to find 'camera' location");

  mvp_position = glGetUniformLocation(program, "MVP");
  WARN(mvp_position != -1, "Failed to find 'MVP' location");
  
  // Setup Vertex Array Object
  // Setup Vertex Buffer Object
  glGenVertexArrays(1, &vertex_array);
  glBindVertexArray(vertex_array);
  ASSERT(vertex_array != 0, "Failed to create vertex array object");
  glGenBuffers(1, &vertex_buffer);
  ASSERT(vertex_buffer != 0, "Failed to create vertex buffer object");
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(position_buffer_data),
	       position_buffer_data, GL_STATIC_DRAW);
  glEnableVertexAttribArray(vposition_position);
  glVertexAttribPointer(vposition_position, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
}

void VolumeObject::Draw(const GLint texture_3d, const GLint texture_tf,
			const float sr, const float step)
{
  check_error_gl("before volume draw");

  glBindVertexArray(vertex_array);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

  glUniformMatrix4fv(mvp_position, 1, GL_FALSE, GetMVPMatrixPtr());
  glUniform1f(samplingrate_location, sr);
  glUniform1f(samplingstep_location, step);
  glUniform3f(camera_position, CameraPos()[0], CameraPos()[1], CameraPos()[2]);
  
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_3D, texture_3d);
  glUniform1i(texture3d_location, 1);

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, texture_tf);    
  glUniform1i(texturetf_location, 2);  

  // We cannot clear color buffer
  // But we must clear depth
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDrawArrays(GL_TRIANGLES, 0, 36);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  check_error_gl("finalize volume draw");
}
