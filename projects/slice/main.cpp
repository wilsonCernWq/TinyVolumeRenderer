// This program is just homework
// So I am using only the most basic functionalities from openGL
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>
#ifdef USE_GLM
# include <glm/glm.hpp>
# include <glm/gtc/matrix_transform.hpp>
# include <glm/gtc/type_ptr.hpp>
#else
# error "GLM is required here"
#endif
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

static const struct
{
  float x, y, z;
  float r, g, b;
} vertices[8] =
{
  {  1, 1, 1, 1.f, 0.f, 0.f }, 
  { -1, 1, 1, 0.f, 1.f, 0.f },
  { -1,-1, 1, 0.f, 0.f, 1.f },
  {  1,-1, 1, 1.f, 1.f, 0.f },
  {  1,-1,-1, 0.f, 1.f, 1.f }, 
  {  1, 1,-1, 1.f, 0.f, 1.f },
  { -1, 1,-1, 1.f, 1.f, 1.f },
  { -1,-1,-1, 0.f, 0.f, 0.f }  
};
static const GLuint indices[36] =
{
  0,1,2, 2,3,0,
  0,3,4, 4,5,0,
  0,5,6, 6,1,0,
  1,6,7, 7,2,1,
  7,4,3, 3,2,7,
  4,7,6, 6,5,4
};

static void error_callback(int error, const char* description)
{
  fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
  { glfwSetWindowShouldClose(window, GLFW_TRUE); }
}

const char* read_file(const char* fname)
{
  std::ifstream file(fname, std::ios::binary | std::ios::ate);
  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);
  char* buffer = new char[size+1]; buffer[size] = '\0';
  if (!file.read(const_cast<char*>(buffer), size)) {
    fprintf(stderr, "Error: Cannot read file %s\n", fname);
    exit(-1);
  }
  return buffer;
}

int main(const int argc, const char** argv)
{
  int data_type, data_size;
  void* data_ptr = NULL;
  ReadVolume(argv[1], data_type, data_size, data_ptr);
  // Initialize GLFW
  glfwSetErrorCallback(error_callback);
  if (!glfwInit()) { exit(EXIT_FAILURE); }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  GLFWwindow* window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
  if (!window) { glfwTerminate(); exit(EXIT_FAILURE); }
  glfwSetKeyCallback(window, key_callback);
  glfwMakeContextCurrent(window);
  gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
  glfwSwapInterval(1); 
  // Setup OpenGL
  glDepthFunc(GL_LESS);
  glDepthMask(GL_TRUE);
  glEnable(GL_DEPTH_TEST); 
  // Compile Simple Shaders
  GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
  const char* vshader_text = read_file("vshader.glsl");
  glShaderSource(vshader, 1, &vshader_text, NULL);
  glCompileShader(vshader);
  GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
  const char* fshader_text = read_file("fshader.glsl");
  glShaderSource(fshader, 1, &fshader_text, NULL);
  glCompileShader(fshader);
  GLuint program = glCreateProgram();
  glAttachShader(program, vshader);
  glAttachShader(program, fshader);
  glLinkProgram(program);
  // Setup Vertex Buffer
  GLuint vertex_array;
  glGenVertexArrays(1, &vertex_array);
  glBindVertexArray(vertex_array);
  GLuint vertex_element;
  glGenBuffers(1, &vertex_element);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_element);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  GLuint vertex_buffer;
  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  GLint vpos_location = glGetAttribLocation(program, "vPos");
  GLint vcol_location = glGetAttribLocation(program, "vCol");
  glEnableVertexAttribArray(vpos_location);
  glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE,
			sizeof(float) * 6, (void*) 0);
  glEnableVertexAttribArray(vcol_location);
  glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
			sizeof(float) * 6, (void*) (sizeof(float) * 3));
  GLint mvp_location = glGetUniformLocation(program, "MVP");
  while (!glfwWindowShouldClose(window))
  {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    float ratio = width / (float) height;
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::vec3 eye = glm::vec3(-20.f, 0.f, 0.f);
    glm::mat4 m = glm::rotate(glm::mat4(1.f), (float) glfwGetTime(), glm::vec3(0,0,1));
    glm::mat4 v = glm::lookAt(eye, glm::vec3(0.f), glm::vec3(0.f,0.f,1.f));
    glm::mat4 p = glm::perspective(30.f/180.f*(float)M_PI, ratio, 1.f, 20.f);
    glm::mat4 mvp = p * v * m;
    glUseProgram(program);
    glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) glm::value_ptr(mvp));
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
