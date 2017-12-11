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
size_t tfn_opacity_dim[2];
size_t tfn_color_dim[2] = {9, 1};
std::vector<float> tfn_opacity_data;
std::vector<float> tfn_color_data =
  {
    0.00f, 0.00f, 1.00f,
    0.00f, 0.25f, 0.75f,
    0.00f, 0.50f, 0.50f,
    0.00f, 0.75f, 0.25f,
    0.00f, 1.00f, 0.00f,
    0.25f, 0.75f, 0.00f,
    0.50f, 0.50f, 0.00f,
    0.75f, 0.25f, 0.00f,
    1.00f, 0.00f, 0.00f,
  };

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