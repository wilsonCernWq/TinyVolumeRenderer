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
