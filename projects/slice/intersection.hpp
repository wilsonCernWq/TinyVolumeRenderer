#pragma once
#include "camera.hpp"
#ifdef USE_GLM
# include <glm/fwd.hpp>
#else
# error "GLM is required here"
#endif
#include <vector>

void IntersectPlane(const float box[24], const int, const int);
void IntersectReset(float);
void IntersectSort();
void IntersectFetch(std::vector<float>&, std::vector<float>&);
void IntersectComputeBox(float clipCoordVertices[24], float&, float&);
