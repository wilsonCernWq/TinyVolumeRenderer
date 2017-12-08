#include "common.h"
#include "global.h"
#include "volume.h"
#include "callback.h"

int main(int argc, const char **argv) {
  //---------------------------------------------------------------------------------------//
  // OpenGL Setup
  //---------------------------------------------------------------------------------------//
  // Create Context
  GLFWwindow *window = CreateWindow();
  check_error_gl("Initialized OpenGL");

  //---------------------------------------------------------------------------------------//
  // OSPRay Setup
  //---------------------------------------------------------------------------------------//
  ospInit(&argc, argv);
  ospLoadModule("tfn");

  //! Init camera and framebuffer
  camera.Init();
  framebuffer.Init(camera.CameraWidth(), camera.CameraHeight());

  //! create world and renderer
  world = ospNewModel();
  renderer = ospNewRenderer("scivis");

  //! setup volume/geometry
  transferFcn = ospNewTransferFunction("piecewise_linear_2d");
  CreateVolume(argc, argv);
  ospCommit(world);

  //! lighting
  OSPLight ambient_light = ospNewLight(renderer, "AmbientLight");
  ospSet1f(ambient_light, "intensity", 0.0f);
  ospCommit(ambient_light);
  OSPLight directional_light = ospNewLight(renderer, "DirectionalLight");
  ospSet1f(directional_light, "intensity", 2.0f);
  ospSetVec3f(directional_light, "direction", osp::vec3f{20.0f, 20.0f, 20.0f});
  ospCommit(directional_light);
  std::vector<OSPLight> light_list{ambient_light, directional_light};
  OSPData lights = ospNewData(light_list.size(), OSP_OBJECT, light_list.data());
  ospCommit(lights);

  //! renderer
  ospSetVec3f(renderer, "bgColor", osp::vec3f{0.5f, 0.5f, 0.5f});
  ospSetData(renderer, "lights", lights);
  ospSetObject(renderer, "model", world);
  ospSetObject(renderer, "camera", camera.OSPRayPtr());
  ospSet1i(renderer, "shadowEnabled", 0);
  ospSet1i(renderer, "oneSidedLighting", 0);
  ospCommit(renderer);

  //---------------------------------------------------------------------------------------//
  // Render
  //---------------------------------------------------------------------------------------//
  RenderWindow(window);

  // exit
  Clean();
  return EXIT_SUCCESS;
}

