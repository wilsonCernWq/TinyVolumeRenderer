// This program is just homework
// So I am using only the most basic functionalities from openGL

#include "glob.hpp"
#include "camera.hpp"
#include "framebuffer.hpp"
#include "screen_object.hpp"
#include "volume_object.hpp"
#include "texture_reader.hpp"

#include <vector>

static FrameBufferObject fbo;
static ScreenObject screen;
static VolumeObject volume;

int main(const int argc, const char** argv)
{
  // Create Context
  GLFWwindow* window = InitWindow();

  // Load Data
  int depth;
  GLuint texture_2d = loadBMP_custom("uvtemplate.bmp");
  GLuint texture_3d = loadRAW_custom(argv[1], depth);
  GLuint texture_tf = loadTFN_custom();
  fprintf(stdout,
	  "[renderer] texture_2d location %i\n"
	  "[renderer] texture_3d location %i\n"
	  "[renderer] texture_tf location %i\n",
	  texture_2d, texture_3d, texture_tf);

  // Initialize Objects
  fbo.Init(CameraWidth(), CameraHeight(), 1);
  screen.Init();
  volume.Init();
  
  // parameters
  const float sr  = 1.0f;
  const float stp = 1.f / depth / sr;
  check_error_gl("start rendering");
  while (!glfwWindowShouldClose(window))
  {
    fbo.BindSingle(0);
    volume.Draw(texture_2d, texture_3d, texture_tf, sr);
    fbo.UnBindAll();

    screen.Draw(fbo.GetColor(0));
    glfwSwapBuffers(window);
    glfwPollEvents();
    check_error_gl("Rendering composer");    
  }

  // Exit
  glfwDestroyWindow(window);
  glfwTerminate();
  return EXIT_SUCCESS;
}
