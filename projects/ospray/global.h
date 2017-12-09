//!
//! This file defines some global variables and all callback functions
//!
#pragma once
#ifndef OSPRAY_GLOBAL_H
#define OSPRAY_GLOBAL_H

#include "common.h"
#include "camera.h"
#include "framebuffer.h"

// camera
extern Camera camera;

// framebuffers
extern Framebuffer framebuffer;

// renderer
extern OSPModel world;
extern OSPRenderer renderer;

// transfer function
extern OSPTransferFunction transferFcn;

// histogram
extern const size_t histXDim, histYDim, histZDim;
extern std::atomic<size_t>* histVolume;
inline std::atomic<size_t>& histVolumeAccess(size_t x, size_t y, size_t z) {
  return histVolume[z * histYDim * histXDim + y * histXDim + x];
}

// cleaning
extern std::vector<std::function<void()>> cleanlist;
void Clean();

#endif //OSPRAY_GLOBAL_H
