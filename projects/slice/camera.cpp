#include "camera.hpp"
#ifdef USE_GLM
# include <glm/glm.hpp>
# include <glm/gtc/matrix_transform.hpp>
# include <glm/gtc/type_ptr.hpp>
#else
# error "GLM is required here"
#endif

//---------------------------------------------------------------------------------------

struct Camera {
  size_t width  = 640, height = 480;
  float  aspect = (float) width / height;
  float  zNear = 1.f, zFar = 50.f;
  float  fovy = 30.f;
  glm::vec3 eye   = glm::vec3(0.f, 0.f, -5.f);
  glm::vec3 focus = glm::vec3(0.f);
  glm::vec3 up    = glm::vec3(0.f,1.f,0.f);
  glm::mat4 view, proj;
  glm::mat4 mvp; // cache
  Camera() { CameraUpdateView(); CameraUpdateProj(width, height); }
};
static Camera camera;

size_t CameraWidth() { return camera.width; }

size_t CameraHeight() { return camera.height; }

void CameraUpdateView()
{
  camera.view = glm::lookAt(camera.eye, camera.focus, camera.up);
}

void CameraUpdateProj(size_t width, size_t height)
{
  camera.aspect = width / (float) height;
  camera.width = width;
  camera.height = height;
  camera.proj = glm::perspective(camera.fovy/180.f*(float)M_PI,
				 camera.aspect, camera.zNear, camera.zFar);
}

const glm::mat4& GetMVPMatrix()
{
  const float angle = 1.f;
  const glm::mat4 m =
    glm::rotate(glm::mat4(1.f), angle, glm::vec3(0,1,0)) *
    glm::rotate(glm::mat4(1.f), angle, glm::vec3(0,0,1));
  camera.mvp = camera.proj * camera.view * m;
  return camera.mvp;
}

const float* GetMVPMatrixPtr()
{
  return glm::value_ptr(GetMVPMatrix());
}
