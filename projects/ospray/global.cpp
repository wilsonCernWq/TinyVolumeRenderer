//
// Created by qwu on 12/7/17.
//

#include "global.h"

// camera
Camera camera;

// framebuffers
Framebuffer framebuffer;

// renderer
OSPModel world = nullptr;
OSPRenderer renderer = nullptr;

// transfer function
OSPTransferFunction transferFcn = nullptr;

// histogram
const size_t histXDim = 64, histYDim = 64, histZDim = 64;
const size_t histCount = histXDim * histYDim * histZDim;
std::atomic<size_t>* histVolume = nullptr;

// cleaning
std::vector<std::function<void()>> cleanlist;
void Clean() {
  std::cout << "[clean] start cleaning" << std::endl;
  camera.Clean();
  framebuffer.Clean();
  if (world != nullptr) {
    ospRelease(world);
    world = nullptr;
  }
  if (renderer != nullptr) {
    ospRelease(renderer);
    renderer = nullptr;
  }
  if (transferFcn != nullptr) {
    ospRelease(transferFcn);
    transferFcn = nullptr;
  }
  for (auto &c : cleanlist) { c(); }
  std::cout << "[clean] done cleaning" << std::endl;
}
