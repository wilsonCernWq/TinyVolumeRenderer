// This program is just homework
// So I am using only the most basic functionalities from openGL
#include <cstdio>
#include <cstdlib>
#include <vector>

#include "glob.hpp"
#include "framebuffer.hpp"
#include "texture_reader.hpp"
#include "screen_object.hpp"
#include "composer_object.hpp"

/////////////////////////////////////////
//    v6----- v5
//   /|      /|
//  v1------v0|
//  | |     | |
//  | |v7---|-|v4
//  |/      |/
//  v2------v3
/////////////////////////////////////////

//static const GLfloat vertex_buffer_data[] = {
//};

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

static GLfloat slice_position_data[] = {
  -1.0f, 1.0f, 0.0f,
   1.0f, 1.0f, 0.0f,
  -1.0f,-1.0f, 0.0f,
   1.0f,-1.0f, 0.0f
};

static ScreenObject quad;
static FrameBufferObject fbo;
static ComposerObject composer;

int main(const int argc, const char** argv)
{
  // Create Context
  GLFWwindow* window = InitWindow();

  // Load data
  GLuint texture_2d = loadBMP_custom("uvtemplate.bmp");
  GLuint texture_3d = loadRAW_custom(argv[1]);
  GLuint texture_tf = loadTFN_custom();
  fprintf(stdout,
	  "[renderer] texture_2d location %i\n"
	  "[renderer] texture_3d location %i\n"
	  "[renderer] texture_tf location %i\n",
	  texture_2d, texture_3d, texture_tf);
  
  // // Compile Simple Shaders
  // GLuint program = LoadProgram("vshader_slice.glsl","fshader_slice.glsl");
  // //GLuint program = LoadProgram("vshader_box.glsl","fshader_box.glsl");
  // ASSERT(program != 0, "Failed to create program");
  
  // // Texture
  // GLint texture2d_location = glGetUniformLocation(program, "tex2d");
  // GLint texture3d_location = glGetUniformLocation(program, "tex3d");
  // GLint texturetf_location = glGetUniformLocation(program, "textf");  
  // WARN(texture2d_location != -1, "Failed to find 'tex2d' location");
  // WARN(texture3d_location != -1, "Failed to find 'tex3d' location");
  // WARN(texturetf_location != -1, "Failed to find 'textf' location");
      
  // // Setup Vertex Buffer
  // GLuint vertex_array;
  // glGenVertexArrays(1, &vertex_array);
  // glBindVertexArray(vertex_array);
  
  // GLuint vertex_buffer[1];
  // glGenBuffers(1, vertex_buffer);
  // glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[0]);

  // // GLuint vertex_buffer[2];
  // // glGenBuffers(2, vertex_buffer);
  // // glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[0]);
  // // glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data),
  // // 	         vertex_buffer_data, GL_STATIC_DRAW);
  // // glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[1]);
  // // glBufferData(GL_ARRAY_BUFFER, sizeof(uv_buffer_data),
  // //             uv_buffer_data, GL_STATIC_DRAW);

  // GLint vposition_location = glGetAttribLocation(program, "vPosition");
  // ASSERT(vposition_location != -1, "Failed to find 'vPosition' location");
  // //GLint vtexcoord_location = glGetAttribLocation(program, "vTexCoord");
  // //ASSERT(vtexcoord_location != -1, "Failed to find 'vTexCoord' location");    
  // //GLint mvp_location = glGetUniformLocation(program, "MVP");
  // //ASSERT(mvp_location != -1, "Failed to find 'MVP' location");

  composer.Init();
  quad.Init();

  check_error_gl("start rendering");
  while (!glfwWindowShouldClose(window))
  {

    fbo.Init(640, 480, 2);
    
    // // bind data
    // glUseProgram(program);
    // glBindVertexArray(vertex_array);

    composer.Bind();
    //glUniformMatrix4fv(mvp_location, 1, GL_FALSE, GetMVPMatrix());

    for (int i = 0; i < 100; ++i) {

      slice_position_data[2 ] = -0.995f + i * 0.01f;
      slice_position_data[5 ] = -0.995f + i * 0.01f;
      slice_position_data[8 ] = -0.995f + i * 0.01f;
      slice_position_data[11] = -0.995f + i * 0.01f;

      fbo.BindSingle(i%2);
      composer.Compose(fbo.GetColor((i+1)%2), texture_3d, texture_tf,
		       slice_position_data, 12);


      // fbo.BindMultiple(i%2,i%2);
      
      // glActiveTexture(GL_TEXTURE0);
      // glBindTexture(GL_TEXTURE_2D, fbo.GetColor((i+1)%2));
      // glUniform1i(texture3d_location, 0);

      // glActiveTexture(GL_TEXTURE1);
      // glBindTexture(GL_TEXTURE_3D, texture_3d);
      // glUniform1i(texture3d_location, 1);

      // glActiveTexture(GL_TEXTURE2);
      // glBindTexture(GL_TEXTURE_2D, texture_tf);    
      // glUniform1i(texturetf_location, 2);

      // glEnableVertexAttribArray(vposition_location);
      // glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[0]);
      // glBufferData(GL_ARRAY_BUFFER, sizeof(slice_position_data),
      // 		   slice_position_data, GL_DYNAMIC_DRAW);
      // glVertexAttribPointer(vposition_location, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

      // check_error_gl("in rendering");
    
      // //glEnableVertexAttribArray(vtexcoord_location);
      // //glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[1]);
      // //glVertexAttribPointer(vtexcoord_location, 2, GL_FLOAT, GL_FALSE, 0, (void*) 0);

      // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);    
      // glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
      fbo.UnBindAll();
    }
    quad.Draw(fbo.GetColor(0));
  
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
