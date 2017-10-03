// This program is just homework
// So I am using only the most basic functionalities from openGL
#include <cstdio>
#include <cstdlib>
#include <vector>

#include "glob.hpp"
#include "framebuffer.hpp"
#include "debug_object.hpp"
#include "texture_reader.hpp"

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

static const GLfloat slice_position_data[] = {
  -1.0f, 1.0f, 0.0f,
   1.0f, 1.0f, 0.0f,
  -1.0f,-1.0f, 0.0f,
   1.0f,-1.0f, 0.0f
};

static DebugQuad quad;
static FrameBufferObject fbo;

int main(const int argc, const char** argv)
{
  // Create Context
  GLFWwindow* window = InitWindow();

  // Load data
  GLuint texture_2d = loadBMP_custom("uvtemplate.bmp");
  GLuint texture_3d = loadRAW_custom(argv[1]);
  fprintf(stdout, "%i, %i\n", texture_2d, texture_3d);
  
  // Compile Simple Shaders
  GLuint program = LoadProgram("vshader_slice.glsl","fshader_slice.glsl");
  //GLuint program = LoadProgram("vshader_box.glsl","fshader_box.glsl");
  ASSERT(program != 0, "Failed to create program");
  
  // Texture
  GLint texture2d_location = glGetUniformLocation(program, "tex2d");
  GLint texture3d_location = glGetUniformLocation(program, "tex3d");  
  WARN(texture2d_location != -1, "Failed to find 'tex2d' location");
  WARN(texture3d_location != -1, "Failed to find 'tex3d' location");
      
  // Setup Vertex Buffer
  GLuint vertex_array;
  glGenVertexArrays(1, &vertex_array);
  glBindVertexArray(vertex_array);
  
  GLuint vertex_buffer[1];
  glGenBuffers(1, vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[0]);

  // GLuint vertex_buffer[2];
  // glGenBuffers(2, vertex_buffer);
  // glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[0]);
  // glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data),
  // 	         vertex_buffer_data, GL_STATIC_DRAW);
  // glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[1]);
  // glBufferData(GL_ARRAY_BUFFER, sizeof(uv_buffer_data),
  //             uv_buffer_data, GL_STATIC_DRAW);

  GLint vposition_location = glGetAttribLocation(program, "vPosition");
  ASSERT(vposition_location != -1, "Failed to find 'vPosition' location");
  //GLint vtexcoord_location = glGetAttribLocation(program, "vTexCoord");
  //ASSERT(vtexcoord_location != -1, "Failed to find 'vTexCoord' location");    
  //GLint mvp_location = glGetUniformLocation(program, "MVP");
  //ASSERT(mvp_location != -1, "Failed to find 'MVP' location");

  //quad.Init();

  //fbo.Init(640, 480, 1);

  check_error_gl("start rendering");
  while (!glfwWindowShouldClose(window))
  {
    //fbo.Bind(1);
    
    // bind data
    glUseProgram(program);
    glBindVertexArray(vertex_array);
    
    //glUniformMatrix4fv(mvp_location, 1, GL_FALSE, GetMVPMatrix());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, texture_3d);    
    glUniform1i(texture3d_location, 0);
    
    glEnableVertexAttribArray(vposition_location);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(slice_position_data),
		 slice_position_data, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(vposition_location, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

    check_error_gl("in rendering");
    
    //glEnableVertexAttribArray(vtexcoord_location);
    //glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[1]);
    //glVertexAttribPointer(vtexcoord_location, 2, GL_FLOAT, GL_FALSE, 0, (void*) 0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    //fbo.Unbind();   
    //quad.Draw(fbo.GetColor(0));
  
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
