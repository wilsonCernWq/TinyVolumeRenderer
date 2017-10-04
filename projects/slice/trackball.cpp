#include "trackball.hpp"
#ifdef USE_GLM
# include <glm/glm.hpp>
# include <glm/gtx/vector_angle.hpp>
# include <glm/gtx/transform.hpp>
#endif
#include <cmath>
  
glm::vec3 Trackball::proj2surf(const float x, const float y) const
{
  float r = x * x + y * y;
  float R = radius * radius;
  float z = r > R / 2 ? R / 2 / sqrt(r) : sqrt(R - r);
  return glm::vec3(x, y, z);
}

void Trackball::Drag(float x, float y)
{
  // get direction
  position = proj2surf(x, y);
  glm::vec3 dir = (invrot ? -1.0f : 1.0f) * glm::normalize(glm::cross(position_prev, position));
  // compute rotation angle
  float angle = glm::angle(glm::normalize(position_prev),
			   glm::normalize(position));
  if (angle < 0.01f) {
    // to prevent position_prev == position, this will cause invalid value
    return;
  }
  else { // compute rotation
    matrix = glm::rotate(angle, dir) * matrix_prev;
  }
}

void Trackball::BeginDrag(float x, float y)
{
  position_prev = proj2surf(x, y);
  position_surf[0] = x;
  position_surf[1] = y;
  matrix_prev = matrix;
}
