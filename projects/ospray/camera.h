#pragma once

#include "common.h"
#include "trackball.h"

class Camera 
{
private:
  glm::vec2 mouse2screen(int x, int y, float width, float height)
  {
    return glm::vec2(2.0f * (float)x / width - 1.0f, 2.0f * (float)y / height - 1.0f);
  }
private:
  size_t width  = 640, height = 480;
  float  aspect = (float) width / height;
  float  zNear = 1.f, zFar = 50.f;
  float  fovy = 30.f;
  glm::vec3 eye   = glm::vec3(0.f,0.f,-50.f); // this trackball requires camera to be
  glm::vec3 focus = glm::vec3(0.f);           // initialized on negtive z axis with
  glm::vec3 up    = glm::vec3(0.f,1.f,0.f);   // y axis as the initial up vector !!!!
  Trackball ball;
  // OSPRay
  OSPCamera ospCamera = nullptr;
public:

  void Clean() 
  {
    if (ospCamera != nullptr) { 
      ospRelease(ospCamera); 
      ospCamera = nullptr;
    }
  }

  void Init() 
  { 
    ospCamera = ospNewCamera("perspective");
    CameraUpdateView(); CameraUpdateProj(this->width, this->height); 
  }

  OSPCamera OSPRayPtr() { return this->ospCamera; }

  size_t CameraWidth()  { return this->width; }
  size_t CameraHeight() { return this->height; }
  float  CameraZNear()  { return this->zNear; }
  float  CameraZFar()   { return this->zFar; }

  void CameraBeginZoom(float x, float y) 
  {
    glm::vec2 p = mouse2screen(x, y, this->width, this->height);
    this->ball.BeginZoom(p.x, p.y);
  }

  void CameraZoom(float x, float y) 
  {
    glm::vec2 p = mouse2screen(x, y, this->width, this->height);
    this->ball.Zoom(p.x, p.y);
    CameraUpdateView();
  }

  void CameraBeginDrag(float x, float y) 
  {
    glm::vec2 p = mouse2screen(x, y, this->width, this->height);
    this->ball.BeginDrag(p.x, p.y);
  }

  void CameraDrag(float x, float y)
  {
    glm::vec2 p = mouse2screen(x, y, this->width, this->height);
    this->ball.Drag(p.x, p.y);
    CameraUpdateView();
  }

  void CameraUpdateView()
  {
    auto dir =
      -glm::vec3(this->ball.Matrix() * glm::vec4(this->eye - this->focus, 0.f));
    auto up  =
      glm::vec3(this->ball.Matrix() * glm::vec4(this->up, 0.f));
    auto pos = (-dir + this->focus);
    ospSetVec3f(ospCamera, "pos", (osp::vec3f&)pos);
    ospSetVec3f(ospCamera, "dir", (osp::vec3f&)dir);
    ospSetVec3f(ospCamera, "up",  (osp::vec3f&)up);
    ospCommit(ospCamera);
  }

  void CameraUpdateProj(size_t width, size_t height)
  {
    this->aspect = width / (float) height;
    this->width  = width;
    this->height = height;
    ospSetf(ospCamera, "aspect", this->aspect);
    ospSetf(ospCamera, "fovy", this->fovy);
    ospCommit(ospCamera);
  }
};
