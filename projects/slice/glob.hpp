#include "comm.hpp"
#ifdef USE_GLM
# include <glm/fwd.hpp>
#else
# error "GLM is required here"
#endif
#include <vector>

GLuint LoadProgram(const char*, const char*);

GLFWwindow* InitWindow();

const glm::mat4& GetMVPMatrix();
const GLfloat*   GetMVPMatrixPtr();

void IntersectPlane(const GLfloat box[24], const glm::ivec2 segment, const float z);
void IntersectReset();
void IntersectSort();
void IntersectFetch(std::vector<GLfloat>&, std::vector<GLfloat>&);
