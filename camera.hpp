//===========================================================================//
//                                                                           //
// Copyright(c) 2018 Qi Wu (Wilson)                                          //
// University of California, Davis                                           //
// MIT Licensed                                                              //
//                                                                           //
//===========================================================================//

#pragma once

#include "opengl.hpp"
#ifdef USE_GLM
#include <glm/fwd.hpp>
#else
#error "GLM is required here"
#endif

void CameraBeginZoom(float x, float y);
void CameraZoom(float x, float y);
void CameraBeginDrag(float x, float y);
void CameraDrag(float x, float y);
void CameraUpdateView();
void CameraUpdateProjection(size_t, size_t);
size_t CameraWidth();
size_t CameraHeight();
float CameraZNear();
float CameraZFar();
const float *CameraPos();
const glm::mat4 &GetProjection();
const glm::mat4 &GetMVMatrix();
const glm::mat4 &GetMVPMatrix();
const float *GetMVPMatrixPtr();
