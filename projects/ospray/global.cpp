//
// Created by qwu on 12/7/17.
//

#include "global.h"

// camera
Camera camera;

// framebuffer
Framebuffer framebuffer;

// volume
Volume volume;

// renderer
OSPModel world = nullptr;
OSPRenderer renderer = nullptr;

// cleaning
std::vector<std::function<void()>> clean_list;
void Clean() {
  std::cout << "[clean] start cleaning" << std::endl;
  camera.Clean();
  framebuffer.Clean();
  volume.Clean();
  if (world != nullptr) {
    ospRelease(world);
    world = nullptr;
  }
  if (renderer != nullptr) {
    ospRelease(renderer);
    renderer = nullptr;
  }
  for (auto &c : clean_list) { c(); }
  std::cout << "[clean] done cleaning" << std::endl;
}
