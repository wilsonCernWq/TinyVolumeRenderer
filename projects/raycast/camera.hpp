#pragma once
#include "comm.hpp"
#ifdef USE_GLM
# include <glm/fwd.hpp>
#else
# error "GLM is required here"
#endif

void CameraBeginZoom(float x, float y);
void CameraZoom(float x, float y);
void CameraBeginDrag(float x, float y);
void CameraDrag(float x, float y);
void CameraUpdateView();
void CameraUpdateProj(size_t, size_t);
size_t CameraWidth();
size_t CameraHeight();
float CameraZNear();
float CameraZFar();
const float* CameraPos();
const glm::mat4& GetProjection();
const glm::mat4& GetMVMatrix();
const glm::mat4& GetMVPMatrix();
const float*     GetMVPMatrixPtr();
