#pragma once
#ifdef USE_GLM
# include <glm/fwd.hpp>
#else
# error "GLM is required here"
#endif

void CameraUpdateView();
void CameraUpdateProj(size_t, size_t);
size_t CameraWidth();
size_t CameraHeight();

const glm::mat4& GetMVPMatrix();
const float*     GetMVPMatrixPtr();
