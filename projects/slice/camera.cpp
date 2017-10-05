#include "camera.hpp"
#include "trackball.hpp"
#ifdef USE_GLM
# include <glm/glm.hpp>
# include <glm/gtc/matrix_transform.hpp>
# include <glm/gtc/type_ptr.hpp>
#else
# error "GLM is required here"
#endif

//---------------------------------------------------------------------------------------

struct Camera {
  size_t width  = 1290, height = 960;
  float  aspect = (float) width / height;
  float  zNear = 1.f, zFar = 50.f;
  float  fovy = 30.f;
  glm::vec3 eye   = glm::vec3(0.f, 0.f, -5.f);
  glm::vec3 focus = glm::vec3(0.f);
  glm::vec3 up    = glm::vec3(0.f,1.f,0.f);
  glm::mat4 view, proj;
  glm::mat4 mv, mvp; // cache
  Camera() { CameraUpdateView(); CameraUpdateProj(width, height); }
};
static Camera camera;
static Trackball ball;

//---------------------------------------------------------------------------------------
// change from mouse coordinate to screen coordinate
static glm::vec2 mouse2screen
(int x, int y, float width, float height)
{
  return glm::vec2(2.0f * (float)x / width - 1.0f, 1.0f - 2.0f * (float)y / height);
}

//---------------------------------------------------------------------------------------

size_t CameraWidth() { return camera.width; }
size_t CameraHeight() { return camera.height; }
float CameraZNear() { return camera.zNear; }
float CameraZFar()  { return camera.zFar; }
 
void CameraBeginZoom(float x, float y) 
{
  glm::vec2 p = mouse2screen(x, y, camera.width, camera.height);
  ball.BeginZoom(p.x, p.y);
}

void CameraZoom(float x, float y) 
{
  glm::vec2 p = mouse2screen(x, y, camera.width, camera.height);
  ball.Zoom(p.x, p.y);
}

void CameraBeginDrag(float x, float y) 
{
  glm::vec2 p = mouse2screen(x, y, camera.width, camera.height);
  ball.BeginDrag(p.x, p.y);
}

void CameraDrag(float x, float y)
{
  glm::vec2 p = mouse2screen(x, y, camera.width, camera.height);
  ball.Drag(p.x, p.y);
}

void CameraUpdateView()
{
  camera.view = glm::lookAt(camera.eye, camera.focus, camera.up);
}

void CameraUpdateProj(size_t width, size_t height)
{
  camera.aspect = width / (float) height;
  camera.width  = width;
  camera.height = height;
  camera.proj = glm::perspective(camera.fovy/180.f*(float)M_PI,
				 camera.aspect, camera.zNear, camera.zFar);
}

const glm::mat4& GetProjection() 
{
  return camera.proj;
}

const glm::mat4& GetMVMatrix()
{ 
  const glm::mat4 m = glm::rotate(glm::mat4(1.f), 0.f, glm::vec3(0,1,0));
  camera.mv = camera.view * ball.Matrix() * m; 
  return camera.mv;
}

const glm::mat4& GetMVPMatrix()
{
  camera.mvp = camera.proj * GetMVMatrix();
  return camera.mvp;
}

const float* GetMVPMatrixPtr()
{
  return glm::value_ptr(GetMVPMatrix());
}
