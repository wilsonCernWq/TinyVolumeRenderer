// This program is just homework
// So I am using only the most basic functionalities from openGL

#include "glob.hpp"
#include "camera.hpp"
#include "intersection.hpp"
#include "framebuffer.hpp"
#include "screen_object.hpp"
#include "composer_object.hpp"
#include "texture_reader.hpp"

#include <vector>

static FrameBufferObject fbo;
static ScreenObject   screen;
static ComposerObject composer;

int main(const int argc, const char** argv)
{
  // Create Context
  GLFWwindow* window = InitWindow();

  // Load Data
  GLuint texture_3d = loadRAW_custom(argv[1]);
  GLuint texture_tf = loadTFN_custom();
  fprintf(stdout,
	  "[renderer] texture_3d location %i\n"
	  "[renderer] texture_tf location %i\n",
	  texture_3d, texture_tf);

  // Initialize Objects
  fbo.Init(640, 480, 2);
  composer.Init();
  screen.Init();
  
  check_error_gl("start rendering");
  while (!glfwWindowShouldClose(window))
  {
    // Compute Vertices
    float min = 10000.f, max = -10000.f;
    GLfloat box_world_vertices[24];
    IntersectComputeBox(box_world_vertices, min, max);

    // Compose Everything
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
      IntersectPlane(box_world_vertices, 0,5, z);
      IntersectPlane(box_world_vertices, 1,6, z);
      IntersectPlane(box_world_vertices, 2,7, z);
      IntersectPlane(box_world_vertices, 3,4, z);
      IntersectPlane(box_world_vertices, 0,1, z);
      IntersectPlane(box_world_vertices, 1,2, z);
      IntersectPlane(box_world_vertices, 2,3, z);
      IntersectPlane(box_world_vertices, 3,0, z);
      IntersectPlane(box_world_vertices, 4,5, z);
      IntersectPlane(box_world_vertices, 5,6, z);
      IntersectPlane(box_world_vertices, 6,7, z);
      IntersectPlane(box_world_vertices, 7,4, z);
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

    // Display Final Image
    screen.Draw(fbo.GetColor(drawBuffer));
    glfwSwapBuffers(window);
    glfwPollEvents();
    check_error_gl("Rendering composer");
    
  }

  // Exit
  glfwDestroyWindow(window);
  glfwTerminate();
  return EXIT_SUCCESS;
}
