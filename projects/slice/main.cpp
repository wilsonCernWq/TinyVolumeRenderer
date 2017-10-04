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

static const GLfloat box_model_vertices[] =
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
    
  composer.Init();
  quad.Init();
  fbo.Init(640, 480, 2);
  
  check_error_gl("start rendering");
  while (!glfwWindowShouldClose(window))
  {
    // compute vertices
    const glm::mat4& mvp = GetMVPMatrix();
    float min = 10000.f, max = -10000.f;
    GLfloat box_world_vertices[24];
    for (int i = 0; i < 8; ++i) {
      glm::vec4 vertex(box_model_vertices[3 * i + 0],
		       box_model_vertices[3 * i + 1],
		       box_model_vertices[3 * i + 2], 1.f);
      vertex = mvp * vertex;      
      box_world_vertices[3 * i + 0] = vertex.x / vertex.w;
      box_world_vertices[3 * i + 1] = vertex.y / vertex.w;
      box_world_vertices[3 * i + 2] = vertex.z / vertex.w;
      min = std::min(min, box_world_vertices[3 * i + 2]);
      max = std::max(max, box_world_vertices[3 * i + 2]);      
    }

    // compose everything
    fbo.Reset();        
    const float stp = 0.001f;
    const float sr  = 1.f / stp;
    int sliceIdx = 0;
    int drawBuffer;
    int readBuffer;
    for (float z = min; z < max; z += stp) // for each slice
    {
      composer.Bind();
      IntersectReset();
      IntersectPlane(box_world_vertices, glm::ivec2(0,5), z);
      IntersectPlane(box_world_vertices, glm::ivec2(1,6), z);
      IntersectPlane(box_world_vertices, glm::ivec2(2,7), z);
      IntersectPlane(box_world_vertices, glm::ivec2(3,4), z);
      IntersectPlane(box_world_vertices, glm::ivec2(0,1), z);
      IntersectPlane(box_world_vertices, glm::ivec2(1,2), z);
      IntersectPlane(box_world_vertices, glm::ivec2(2,3), z);
      IntersectPlane(box_world_vertices, glm::ivec2(3,0), z);
      IntersectPlane(box_world_vertices, glm::ivec2(4,5), z);
      IntersectPlane(box_world_vertices, glm::ivec2(5,6), z);
      IntersectPlane(box_world_vertices, glm::ivec2(6,7), z);
      IntersectPlane(box_world_vertices, glm::ivec2(7,4), z);
      IntersectSort();
      std::vector<GLfloat> slice_position;
      std::vector<GLfloat> slice_texcoord;
      IntersectFetch(slice_position, slice_texcoord);
      int drawBuffer = sliceIdx % 2;
      int readBuffer = (sliceIdx + 1) % 2;
      ++sliceIdx;
      fbo.BindSingle(drawBuffer);
      composer.Compose(fbo.GetColor(readBuffer), texture_3d, texture_tf, sr / 1000.f,
    		       slice_position.data(), slice_texcoord.data(),
    		       slice_position.size());
      fbo.UnBindAll();
    }

    // Display final image
    quad.Draw(fbo.GetColor(drawBuffer));
    glfwSwapBuffers(window);
    glfwPollEvents();
    check_error_gl("Rendering composer");
    
  }
  
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
