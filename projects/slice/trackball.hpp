/**
 * This file defines the trackball class
 * which is independent to the project itself.
 */
#pragma once
#ifndef _TRACKBALL_H_
#define _TRACKBALL_H_
#ifdef USE_GLM
# include <glm/glm.hpp>
# include <glm/gtx/vector_angle.hpp>
# include <glm/gtx/transform.hpp>
#endif

//
// following the implementation of
// http://image.diku.dk/research/trackballs/index.html
//
class Trackball {
private:
  bool  invrot = false; // inverse rotation
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
  Trackball(bool i) : invrot(i) {}

  void SetRadius(const float r) { radius = r; }
  void SetInverseMode(bool r) { invrot = r; }

  /**
   * @brief BeginDrag/Zoom: initialize drag/zoom
   * @param x: previous x position
   * @param y: previous y position
   */
  void BeginDrag(float x, float y);
  void BeginZoom(float x, float y) {
    zoom_prev = y;
  }

  /**
   * @brief Drag/Zoom: execute Drag/Zoom
   * @param x: current x position
   * @param y: current y position
   */
  void Drag(float x, float y);
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
  glm::vec3 proj2surf(const float x, const float y) const;    
};

#endif//_TRACKBALL_H_
