#include "common/common.h"
#include "global.h"
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
  // initialize ospray and modules
  ospInit(&argc, argv);
  ospLoadModule("tfn");

  // initialize camera and framebuffer
  camera.Init();
  framebuffer.Init(camera.CameraWidth(), camera.CameraHeight());
  volume.Init(argc, argv);

  // create world and renderer
  world = ospNewModel();
  ospAddVolume(world, volume.OSPRayPtr());
  ospCommit(world);
  renderer = ospNewRenderer("scivis");

  // lighting
  OSPLight ambient_light = ospNewLight(renderer, "AmbientLight");
  ospSet1f(ambient_light, "intensity", 0.9f);
  ospCommit(ambient_light);
  OSPLight directional_light = ospNewLight(renderer, "DirectionalLight");
  ospSetVec3f(directional_light, "direction", osp::vec3f{-0.93f, -0.54f, -0.605f});
  ospSet1f(directional_light, "intensity", 0.25f);
  ospCommit(directional_light);
  OSPLight sun_light = ospNewLight(renderer, "DirectionalLight");
  ospSetVec3f(sun_light, "direction", osp::vec3f{0.462f, -1.0f, -0.1f});
  ospSet1f(sun_light, "angularDiameter", 0.53f);
  ospSet1f(sun_light, "intensity", 1.5f);
  ospCommit(sun_light);
  std::vector<OSPLight> light_list{ambient_light, directional_light, sun_light};
  OSPData lights = ospNewData(light_list.size(), OSP_OBJECT, light_list.data());
  ospCommit(lights);

  // renderer
  ospSetVec3f(renderer, "bgColor", osp::vec3f{0.5f, 0.5f, 0.5f});
  ospSetData(renderer, "lights", lights);
  ospSetObject(renderer, "model", world);
  ospSetObject(renderer, "camera", camera.OSPRayPtr());
  ospSet1i(renderer, "shadowEnabled", true);
  ospSet1i(renderer, "oneSidedLighting", false);
  ospSet1f(renderer, "varianceThreshold", 0.1f);
  ospCommit(renderer);

  //---------------------------------------------------------------------------------------//
  // Render
  //---------------------------------------------------------------------------------------//
  RenderWindow(window);

  // exit
  Clean();
  return EXIT_SUCCESS;
}

