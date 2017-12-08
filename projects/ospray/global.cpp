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

std::vector<float> hist;
const int hist_xdim = 64, hist_ydim = 64;

//! cleaning
std::vector<std::function<void()>> cleanlist;