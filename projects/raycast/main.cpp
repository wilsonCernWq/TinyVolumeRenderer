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
  // Parse command
  float sr = 1.f;
  if (argc < 2) {
    fprintf(stderr, "Error: insufficient input\n");
    exit(-1);
  }
  const char *jsonfile = argv[1];
  for (int i = 2; i < argc; ++i) {
    std::string str(argv[i]);
    if (str.compare("-sr") == 0) {
      sr = std::atof(argv[++i]);
    } else {
      fprintf(stderr,"Error option: %s\n", argv[i]);
      exit(-1);
    }
  }
  
  // Create Context
  GLFWwindow* window = InitWindow();

  // Load Data
  int depth;
  GLuint texture_3d = loadRAW_custom(jsonfile, depth);
  GLuint texture_tf = loadTFN_custom();
  fprintf(stdout,
	  "[renderer] texture_3d location %i\n"
	  "[renderer] texture_tf location %i\n",
	  texture_3d, texture_tf);

  // Initialize Objects
  fbo.Init(CameraWidth(), CameraHeight(), 1);
  screen.Init();
  volume.Init();
  
  // parameters
  const float stp = 1.f / depth / sr; // raycasting step
  check_error_gl("start rendering");  
  while (!glfwWindowShouldClose(window))
  {
    // render objects
    volume.Draw(texture_3d, texture_tf, sr * 2.f, stp);
    // render GUI objects
    RenderGUI();
    // swap frame
    glfwSwapBuffers(window);
    glfwPollEvents();
    check_error_gl("Rendering composer");    
  }
  
  // Exit
  ShutdownWindow(window);
  glfwTerminate();
  return EXIT_SUCCESS;
}
