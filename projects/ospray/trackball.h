/**
 * This file defines the trackball class
 * which is independent to the project itself.
 */
#pragma once
#ifndef _TRACKBALL_H_
#define _TRACKBALL_H_
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
//
// following the implementation of
// http://image.diku.dk/research/trackballs/index.html
//
// NOTE:
// This trackball has some camera requirements:
// 1) Camera should be located on negtive z axis initially
// 2) Use y axis as the initial up vector
//
class Trackball {
private:
  bool  inverse_rotate = true; // inverse rotation
  float radius = 1.0f;
  glm::mat4 matrix      = glm::mat4(1.0f);
  glm::mat4 matrix_prev = glm::mat4(1.0f);
  glm::vec3 position;
  glm::vec3 position_prev;
  float position_surf[2];
  float zoom = 1.f;
  float zoom_prev;
public:
  /** constractors */
  Trackball() {}

  void SetRadius(const float r) { radius = r; }
  void SetInverseRotateMode(bool r) { inverse_rotate = r; }

  /**
   * @brief BeginDrag/Zoom: initialize drag/zoom
   * @param x: previous x position
   * @param y: previous y position
   */
  void BeginDrag(float x, float y)
  {
    position_prev = proj2surf(x, y);
    position_surf[0] = x;
    position_surf[1] = y;
    matrix_prev = matrix;
  }

  void BeginZoom(float x, float y) {
    zoom_prev = y;
  }

  /**
   * @brief Drag/Zoom: execute Drag/Zoom
   * @param x: current x position
   * @param y: current y position
   */
  void Drag(float x, float y)
  {
    // get direction
    position = proj2surf(x, y);
    glm::vec3 dir = glm::normalize(glm::cross(position_prev, position));
    if (inverse_rotate) dir *= -1.0f;
    // compute rotation angle
    float angle = acos(glm::dot(glm::normalize(position_prev), glm::normalize(position)));
    if (angle < 0.001f) {
      // to prevent position_prev == position, this will cause invalid value
      return;
    }
    else { // compute rotation
      matrix = glm::rotate(matrix_prev, angle, dir);
    }
  }

  void Zoom(float x, float y) {
    zoom += (y - zoom_prev);
    zoom_prev = y;
  }
    
  /**
   * @brief matrix: trackball matrix accessor
   * @return current trackball matrix
   */
  glm::mat4 Matrix() { return glm::scale(matrix, glm::vec3(zoom)); }

  void Reset() { matrix = glm::mat4(1.0f); zoom = 1.0f; }
  void Reset(const glm::mat4& m) { matrix = m; }

private:
  /**
   * @brief proj2surf: project (x,y) mouse pos on surface
   * @param x: X position
   * @param y: Y position
   * @return projected position
   */
  glm::vec3 proj2surf(const float x, const float y) const
  {
    float r = x * x + y * y;
    float R = radius * radius;
    float z = r > R / 2 ? R / 2 / sqrt(r) : sqrt(R - r);
    return glm::vec3(x, y, z);
  }
};

#endif//_TRACKBALL_H_
