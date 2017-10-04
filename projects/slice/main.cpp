// This program is just homework
// So I am using only the most basic functionalities from openGL

#include "glob.hpp"
#include "framebuffer.hpp"
#include "texture_reader.hpp"
#include "screen_object.hpp"
#include "composer_object.hpp"

#ifdef USE_GLM
# include <glm/glm.hpp>
# include <glm/gtc/matrix_transform.hpp>
# include <glm/gtc/type_ptr.hpp>
#else
# error "GLM is required here"
#endif

#include <cstdio>
#include <cstdlib>
#include <vector>

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

static GLfloat slice_position_data[] = {
   0.0f, 0.0f, 0.0f,
  -0.5f, 0.5f, 0.0f,
   0.5f, 0.5f, 0.0f,
   0.5f,-0.5f, 0.0f,
  -0.5f,-0.5f, 0.0f,
  -0.5f, 0.5f, 0.0f,
};
static GLfloat slice_texcoord_data[] = {
  0.5f, 0.5f, 0.0f,
  0.0f, 1.0f, 0.0f,
  1.0f, 1.0f, 0.0f,
  1.0f, 0.0f, 0.0f,
  0.0f, 0.0f, 0.0f,
  0.0f, 1.0f, 0.0f
};

static const GLfloat box_position_data_local[] =
{
   1, 1, 1,
  -1, 1, 1,
  -1,-1, 1,
   1,-1, 1,
   1,-1,-1,
   1, 1,-1,
  -1, 1,-1,
  -1,-1,-1
};

static const GLfloat box_color_data[] =
{
  1.f, 0.f, 0.f,
  0.f, 1.f, 0.f,
  0.f, 0.f, 1.f,
  1.f, 1.f, 0.f,
  0.f, 1.f, 1.f,
  1.f, 0.f, 1.f,
  1.f, 1.f, 1.f,
  0.f, 0.f, 0.f
};

static const GLuint box_index_data[] =
{
  0,1,2, 2,3,0,
  0,3,4, 4,5,0,
  0,5,6, 6,1,0,
  1,6,7, 7,2,1,
  7,4,3, 3,2,7,
  4,7,6, 6,5,4
};

static ScreenObject quad;
static FrameBufferObject fbo;
static ComposerObject composer;

int main(const int argc, const char** argv)
{
  // Create Context
  GLFWwindow* window = InitWindow();

  // Load data
  GLuint texture_3d = loadRAW_custom(argv[1]);
  GLuint texture_tf = loadTFN_custom();
  fprintf(stdout,
	  "[renderer] texture_3d location %i\n"
	  "[renderer] texture_tf location %i\n",
	  texture_3d, texture_tf);
  
  // Compile Simple Shaders
  GLuint program = LoadProgram("vshader_box.glsl","fshader_box.glsl");
  ASSERT(program != 0, "Failed to create program");
  
  // // Texture
  // GLint texture2d_location = glGetUniformLocation(program, "tex2d");
  // WARN(texture2d_location != -1, "Failed to find 'tex2d' location");
  // GLint texture3d_location = glGetUniformLocation(program, "tex3d");
  // WARN(texture3d_location != -1, "Failed to find 'tex3d' location");
  // GLint texturetf_location = glGetUniformLocation(program, "textf");
  // WARN(texturetf_location != -1, "Failed to find 'textf' location");
      
  // // Setup Vertex Buffer
  // GLuint vertex_array;
  // glGenVertexArrays(1, &vertex_array);
  // glBindVertexArray(vertex_array);

  // GLuint vertex_element;
  // glGenBuffers(1, &vertex_element);
  // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_element);
  // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(box_index_data),
  // 	       box_index_data, GL_STATIC_DRAW);
  
  // GLuint vertex_buffer[2];
  // glGenBuffers(2, vertex_buffer);
  // glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[0]);

  // glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[1]);
  // glBufferData(GL_ARRAY_BUFFER, sizeof(box_color_data),
  // 	       box_color_data, GL_STATIC_DRAW);

  // GLint vposition_location = glGetAttribLocation(program, "vPosition");
  // ASSERT(vposition_location != -1, "Failed to find 'vPosition' location");
  // GLint vcolor_location    = glGetAttribLocation(program, "vColor");
  // ASSERT(vcolor_location    != -1, "Failed to find 'vColor'    location");    

  composer.Init();
  quad.Init();

  check_error_gl("start rendering");
  while (!glfwWindowShouldClose(window))
  {

    // compute vertices
    const glm::mat4& mvp = GetMVPMatrix();
    float min = 10000.f, max = -10000.f;
    GLfloat box_position_data[24];
    for (int i = 0; i < 8; ++i) {
      glm::vec4 vertex(box_position_data_local[3 * i + 0],
		       box_position_data_local[3 * i + 1],
		       box_position_data_local[3 * i + 2], 1.f);
      vertex = mvp * vertex;      
      box_position_data[3 * i + 0] = vertex.x / vertex.w;
      box_position_data[3 * i + 1] = vertex.y / vertex.w;
      box_position_data[3 * i + 2] = vertex.z / vertex.w;
      min = std::min(min, box_position_data[3 * i + 2]);
      max = std::max(max, box_position_data[3 * i + 2]);      
    }
    
    // // pass into program
    // glUseProgram(program);
    // glBindVertexArray(vertex_array);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_element);
        
    // glEnableVertexAttribArray(vposition_location);
    // glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[0]);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(box_position_data),
    // 		 box_position_data, GL_DYNAMIC_DRAW);
    // glVertexAttribPointer(vposition_location, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

    // glEnableVertexAttribArray(vcolor_location);
    // glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[1]);
    // glVertexAttribPointer(vcolor_location, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);    
    // glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    // check_error_gl("Rendering box");
    
    fbo.Init(640, 480, 2);
    composer.Bind();
    for (int i = 0; i < 100; ++i) // for each slice
    {
      float z;
      
      z = min + i * (max - min) * 0.01f;
      
      IntersectReset();
      IntersectPlane(box_position_data, glm::ivec2(0,5), z);
      IntersectPlane(box_position_data, glm::ivec2(1,6), z);
      IntersectPlane(box_position_data, glm::ivec2(2,7), z);
      IntersectPlane(box_position_data, glm::ivec2(3,4), z);
      IntersectPlane(box_position_data, glm::ivec2(0,1), z);
      IntersectPlane(box_position_data, glm::ivec2(1,2), z);
      IntersectPlane(box_position_data, glm::ivec2(2,3), z);
      IntersectPlane(box_position_data, glm::ivec2(3,0), z);
      IntersectPlane(box_position_data, glm::ivec2(4,5), z);
      IntersectPlane(box_position_data, glm::ivec2(5,6), z);
      IntersectPlane(box_position_data, glm::ivec2(6,7), z);
      IntersectPlane(box_position_data, glm::ivec2(7,4), z);
      IntersectSort();

      std::vector<GLfloat> slice_position;
      std::vector<GLfloat> slice_texcoord;
      IntersectFetch(slice_position, slice_texcoord);
      
      // z = -0.995f + i * 0.01f;      
      // slice_position_data[2 ] = z;
      // slice_position_data[5 ] = z;
      // slice_position_data[8 ] = z;
      // slice_position_data[11] = z;
      // slice_position_data[14] = z;
      // slice_position_data[17] = z;
      // slice_texcoord_data[2 ] = z * 0.5f + 0.5f;
      // slice_texcoord_data[5 ] = z * 0.5f + 0.5f;
      // slice_texcoord_data[8 ] = z * 0.5f + 0.5f;
      // slice_texcoord_data[11] = z * 0.5f + 0.5f;
      // slice_texcoord_data[14] = z * 0.5f + 0.5f;
      // slice_texcoord_data[17] = z * 0.5f + 0.5f;
      
      fbo.BindSingle(i%2);
      composer.Compose(fbo.GetColor((i+1) % 2), texture_3d, texture_tf,
    		       slice_position.data(), slice_texcoord.data(),
    		       slice_position.size());
      fbo.UnBindAll();
    }
    quad.Draw(fbo.GetColor(0));
    check_error_gl("Rendering composer");
    
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
