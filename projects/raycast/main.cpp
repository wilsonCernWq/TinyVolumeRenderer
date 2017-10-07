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
  int depth;
  GLuint texture_3d = loadRAW_custom(argv[1], depth);
  GLuint texture_tf = loadTFN_custom();
  fprintf(stdout,
	  "[renderer] texture_3d location %i\n"
	  "[renderer] texture_tf location %i\n",
	  texture_3d, texture_tf);

  // Initialize Objects
  fbo.Init(CameraWidth(), CameraHeight(), 2);
  composer.Init();
  screen.Init();

  // parameters
  const float sr  = 1.0f;
  const float stp = 1.f / depth / sr;
  check_error_gl("start rendering");
  while (!glfwWindowShouldClose(window))
  {
    // Compute Vertices
    float cameraCoordZMin, cameraCoordZMax;
    GLfloat boxClipCoordPosition[24];
    IntersectComputeBox(boxClipCoordPosition, cameraCoordZMin, cameraCoordZMax);

    // Compose Everything
    fbo.Reset();      
    int sliceIdx = 0;
    int drawBuffer;
    int readBuffer;
    for (float z = cameraCoordZMax; z >= cameraCoordZMin; z -= stp) // for each slice
    { 
      composer.Bind();
      IntersectReset(z);
      IntersectPlane(boxClipCoordPosition, 0,5);
      IntersectPlane(boxClipCoordPosition, 1,6);
      IntersectPlane(boxClipCoordPosition, 2,7);
      IntersectPlane(boxClipCoordPosition, 3,4);
      IntersectPlane(boxClipCoordPosition, 0,1);
      IntersectPlane(boxClipCoordPosition, 1,2);
      IntersectPlane(boxClipCoordPosition, 2,3);
      IntersectPlane(boxClipCoordPosition, 3,0);
      IntersectPlane(boxClipCoordPosition, 4,5);
      IntersectPlane(boxClipCoordPosition, 5,6);
      IntersectPlane(boxClipCoordPosition, 6,7);
      IntersectPlane(boxClipCoordPosition, 7,4);
      IntersectSort();
      std::vector<GLfloat> slice_position;
      std::vector<GLfloat> slice_texcoord;
      IntersectFetch(slice_position, slice_texcoord);
      int drawBuffer = sliceIdx % 2;
      int readBuffer = (sliceIdx + 1) % 2;
      ++sliceIdx;
      fbo.BindSingle(drawBuffer);
      composer.Compose(fbo.GetColor(readBuffer), texture_3d, texture_tf, sr * 2.f,
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
