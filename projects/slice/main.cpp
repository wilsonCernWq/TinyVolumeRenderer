// This program is just homework
// So I am using only the most basic functionalities from openGL

#include "glob.hpp"
#include "camera.hpp"
#include "intersection.hpp"
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
  fbo.Init(CameraWidth(), CameraHeight(), 2);
  screen.Init();
  volume.Init();

  // parameters
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
      volume.Bind();
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
      drawBuffer = sliceIdx % 2;
      readBuffer = (sliceIdx + 1) % 2;
      ++sliceIdx;
      fbo.BindSingle(drawBuffer);
      volume.Compose(fbo.GetColor(readBuffer), texture_3d, texture_tf, sr,
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
