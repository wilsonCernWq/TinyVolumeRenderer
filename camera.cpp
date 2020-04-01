//===========================================================================//
//                                                                           //
// Copyright(c) 2018 Qi Wu (Wilson)                                          //
// University of California, Davis                                           //
// MIT Licensed                                                              //
//                                                                           //
//===========================================================================//

#include "camera.hpp"
#include "trackball.hpp"
#ifdef USE_GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#else
#error "GLM is required here"
#endif

struct Camera {
public:
    size_t    width = 640, height = 480;
    float     aspect = (float)width / height;
    float     zNear = 1.f, zFar = 50.f;
    float     fovy    = 30.f;
    glm::vec3 eye     = glm::vec3(0.f, 0.f, -5.f);
    glm::vec3 focus   = glm::vec3(0.f);
    glm::vec3 up      = glm::vec3(0.f, 1.f, 0.f);
    glm::mat4 view    = glm::identity<glm::mat4>();
    glm::mat4 project = glm::identity<glm::mat4>();
    Trackball ball;
    // cache variable
    glm::mat4 mv     = glm::identity<glm::mat4>(); // cache
    glm::mat4 mvp    = glm::identity<glm::mat4>();
    glm::vec3 campos = glm::zero<glm::vec3>();

public:
    Camera()
    {
        CameraUpdateView();
        CameraUpdateProjection(width, height);
    }
};
static Camera camera; // NOLINT(cert-err58-cpp)

// change from mouse coordinate to screen coordinate
static glm::vec2
mouse2screen(int x, int y, float width, float height)
{
    return glm::vec2(2.0f * (float)x / width - 1.0f, 2.0f * (float)y / height - 1.0f);
}

size_t
CameraWidth()
{
    return camera.width;
}
size_t
CameraHeight()
{
    return camera.height;
}
float
CameraZNear()
{
    return camera.zNear;
}
float
CameraZFar()
{
    return camera.zFar;
}
const float*
CameraPos()
{
    camera.campos = glm::vec3(camera.ball.Matrix() * glm::vec4(camera.eye - camera.focus, 0.f)) + camera.focus;
    return glm::value_ptr(camera.campos);
}

void
CameraBeginZoom(float x, float y)
{
    glm::vec2 p = mouse2screen(static_cast<int>(x), static_cast<int>(y), camera.width, camera.height);
    camera.ball.BeginZoom(p.x, p.y);
}

void
CameraZoom(float x, float y)
{
    glm::vec2 p = mouse2screen(static_cast<int>(x), static_cast<int>(y), camera.width, camera.height);
    camera.ball.Zoom(p.x, p.y);
    CameraUpdateView();
}

void
CameraBeginDrag(float x, float y)
{
    glm::vec2 p = mouse2screen(static_cast<int>(x), static_cast<int>(y), camera.width, camera.height);
    camera.ball.BeginDrag(p.x, p.y);
}

void
CameraDrag(float x, float y)
{
    glm::vec2 p = mouse2screen(static_cast<int>(x), static_cast<int>(y), camera.width, camera.height);
    camera.ball.Drag(p.x, p.y);
    CameraUpdateView();
}

void
CameraUpdateView()
{
    glm::vec3 dir = glm::vec3(camera.ball.Matrix() * glm::vec4(camera.eye - camera.focus, 0.f));
    glm::vec3 up  = glm::vec3(camera.ball.Matrix() * glm::vec4(camera.up, 0.f));
    camera.view   = glm::lookAt(dir + camera.focus, camera.focus, up);
}

void
CameraUpdateProjection(size_t width, size_t height)
{
    camera.aspect  = width / (float)height;
    camera.width   = width;
    camera.height  = height;
    camera.project = glm::perspective(camera.fovy / 180.f * (float)M_PI, camera.aspect, camera.zNear, camera.zFar);
}

const glm::mat4&
GetProjection()
{
    return camera.project;
}

const glm::mat4&
GetMVMatrix()
{
    camera.mv = camera.view * glm::mat4(1.0f);
    return camera.mv;
}

const glm::mat4&
GetMVPMatrix()
{
    camera.mvp = camera.project * GetMVMatrix();
    return camera.mvp;
}

const float*
GetMVPMatrixPtr()
{
    return glm::value_ptr(GetMVPMatrix());
}
