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
  GLFWwindow* window = InitWindow();
  // Compile Simple Shaders
  GLuint program = LoadProgram("vshader.glsl","fshader.glsl");
  // Texture
  GLuint texture = loadBMP_custom("uvtemplate.bmp");
  GLint texture_location = glGetUniformLocation(program, "tex");
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);    
  glUniform1i(texture_location, 0);
    
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
  GLint vtexcoord_location = glGetAttribLocation(program, "vTexCoord");

  glEnableVertexAttribArray(vposition_location);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[0]);
  glVertexAttribPointer(vposition_location, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
  
  glEnableVertexAttribArray(vtexcoord_location);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[1]);
  glVertexAttribPointer(vtexcoord_location, 2, GL_FLOAT, GL_FALSE, 0, (void*) 0);
  
  GLint mvp_location = glGetUniformLocation(program, "MVP");
  
  while (!glfwWindowShouldClose(window))
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glUseProgram(program);
    glUniformMatrix4fv(mvp_location, 1, GL_FALSE, GetMVPMatrix());
     
    glDrawArrays(GL_TRIANGLES, 0, 12 * 3); // 12*3 indices starting at 0 -> 12 triangles
    
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
