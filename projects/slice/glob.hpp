#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#ifdef USE_GLM
# include <glm/fwd.hpp>
#else
# error "GLM is required here"
#endif

static void _glCheckError(const char* file, int line, const char* comment);

#ifndef NDEBUG
# define check_error_gl(x) _glCheckError(__FILE__, __LINE__, x)
#else
# define check_error_gl() ((void)0)
#endif

GLuint LoadProgram(const char*, const char*);

GLFWwindow* InitWindow();

const GLfloat* GetMVPMatrix();
