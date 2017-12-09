//!
//! This file defines some global variables and all callback functions
//!
#pragma once
#ifndef OSPRAY_GLOBAL_H
#define OSPRAY_GLOBAL_H

#include "common/common.h"
#include "common/camera.h"
#include "common/framebuffer.h"

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
extern const size_t histXDim, histYDim, histZDim, histCount;
extern std::atomic<size_t>* histVolume;
inline size_t histIdx(size_t x, size_t y, size_t z) { return z * histYDim * histXDim + y * histXDim + x; }

// cleaning
extern std::vector<std::function<void()>> cleanlist;
void Clean();

#endif //OSPRAY_GLOBAL_H
