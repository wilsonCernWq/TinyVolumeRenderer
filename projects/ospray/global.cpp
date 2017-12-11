//
// Created by qwu on 12/7/17.
//

#include "global.h"
#include <thread>

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

// OSPRay
static std::thread *osprayThread = nullptr;
static std::atomic<bool> osprayStop(false);
static std::atomic<bool> osprayClear(false);

void StartOSPRay() {
  osprayStop = false;
  osprayThread = new std::thread([=] {
    while (!osprayStop) {
      if (osprayClear) { framebuffer.CleanBuffer();osprayClear = false; }
      ospRenderFrame(framebuffer.OSPRayPtr(), renderer, OSP_FB_COLOR | OSP_FB_ACCUM);
      framebuffer.Swap();
    }
  });
}

void StopOSPRay() { osprayStop = true; osprayThread->join(); }

void ClearOSPRay() { osprayClear = true; }

void ResizeOSPRay(int width, int height)
{
  StopOSPRay();
  framebuffer.Resize((size_t)width, (size_t)height);
  StartOSPRay();
}

void UploadOSPRay()
{
  framebuffer.Upload();
}