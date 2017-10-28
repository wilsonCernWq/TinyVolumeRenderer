#include "comm.hpp"
#ifdef USE_GLM
# include <glm/fwd.hpp>
#else
# error "GLM is required here"
#endif

GLuint LoadProgram(const char*, const char*);

void RenderGUI(GLuint);

GLFWwindow* InitWindow();

void ShutdownWindow(GLFWwindow*);
