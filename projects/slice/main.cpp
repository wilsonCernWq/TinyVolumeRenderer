// This program is just homework
// So I am using only the most basic functionalities from openGL
#include <cstdio>
#include <cstdlib>
#include <vector>

#include "glob.hpp"
#include "texture.hpp"
#include "volume_reader.hpp"

/////////////////////////////////////////
//    v6----- v5
//   /|      /|
//  v1------v0|
//  | |     | |
//  | |v7---|-|v4
//  |/      |/
//  v2------v3
/////////////////////////////////////////

static const GLfloat vertex_buffer_data[] = {
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
static const GLfloat uv_buffer_data[] = {
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


int main(const int argc, const char** argv)
{
  // int data_type, data_size;
  // void* data_ptr = NULL;
  // ReadVolume(argv[1], data_type, data_size, data_ptr);

  // Create Context
  GLFWwindow* window = InitWindow();

  // Compile Simple Shaders
  GLuint program = LoadProgram("vshader.glsl","fshader.glsl");
  ASSERT(program != 0, "Failed to create program");
  
  // Texture
  GLuint texture = loadBMP_custom("uvtemplate.bmp");
  GLint texture_location = glGetUniformLocation(program, "tex");  
  ASSERT(texture_location != -1, "Failed to find 'tex' location");
      
  // Setup Vertex Buffer
  GLuint vertex_array;
  glGenVertexArrays(1, &vertex_array);
  glBindVertexArray(vertex_array);
  
  GLuint vertex_buffer[2];
  glGenBuffers(2, vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data),
	       vertex_buffer_data, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(uv_buffer_data),
	       uv_buffer_data, GL_STATIC_DRAW);
  
  GLint vposition_location = glGetAttribLocation(program, "vPosition");
  ASSERT(vposition_location != -1, "Failed to find 'vPosition' location");
  GLint vtexcoord_location = glGetAttribLocation(program, "vTexCoord");
  ASSERT(vtexcoord_location != -1, "Failed to find 'vTexCoord' location");    
  GLint mvp_location = glGetUniformLocation(program, "MVP");
  ASSERT(mvp_location != -1, "Failed to find 'MVP' location");

  // FBO
  size_t fbo_width = 640, fbo_height = 480;
  GLuint FramebufferName;
  glGenFramebuffers(1, &FramebufferName);
  glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
  GLuint fboColorBuffer;
  glGenTextures(1, &fboColorBuffer);
  glBindTexture(GL_TEXTURE_2D, fboColorBuffer);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fbo_width, fbo_height,
	       0, GL_RGB, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fboColorBuffer, 0);
  GLuint fboDepthBuffer;
  glGenRenderbuffers(1, &fboDepthBuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, fboDepthBuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, fbo_width, fbo_height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
			    GL_RENDERBUFFER, fboDepthBuffer);
  GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
  glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  { fprintf(stderr, "FBO Incomplete!\n"); exit(-1); }

  while (!glfwWindowShouldClose(window))
  {    
    glUseProgram(program);
    glUniformMatrix4fv(mvp_location, 1, GL_FALSE, GetMVPMatrix());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);    
    glUniform1i(texture_location, 0);
    glEnableVertexAttribArray(vposition_location);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[0]);
    glVertexAttribPointer(vposition_location, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
    
    glEnableVertexAttribArray(vtexcoord_location);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[1]);
    glVertexAttribPointer(vtexcoord_location, 2, GL_FLOAT, GL_FALSE, 0, (void*) 0);

    //glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);    
    //glDrawArrays(GL_TRIANGLES, 0, 12 * 3); // 12*3 indices starting at 0 -> 12 triangles

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 12 * 3); // 12*3 indices starting at 0 -> 12 triangles

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
