//!
//! This file defines some global variables and all callback functions
//!
#pragma once
#ifndef OSPRAY_GLOBAL_H
#define OSPRAY_GLOBAL_H

#include "common/common.h"
#include "common/camera.h"
#include "common/framebuffer.h"
#include "common/volume.h"

// camera
extern Camera camera;

// framebuffer
extern Framebuffer framebuffer;

// volume
extern Volume volume;
extern std::vector<float> tfn_opacity_data;
extern std::vector<float> tfn_color_data;
extern size_t tfn_opacity_dim[2];
extern size_t tfn_color_dim[2];
// renderer
extern OSPModel world;
extern OSPRenderer renderer;

// cleaning
extern std::vector<std::function<void()>> clean_list;
void Clean();

// ospray
void StartOSPRay();
void StopOSPRay();
void ClearOSPRay();
void ResizeOSPRay(int width, int height);
void UploadOSPRay();

#endif //OSPRAY_GLOBAL_H
