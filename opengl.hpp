//===========================================================================//
//                                                                           //
// Copyright(c) 2018 Qi Wu (Wilson)                                          //
// University of California, Davis                                           //
// MIT Licensed                                                              //
//                                                                           //
//===========================================================================//

#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

#include <glad/glad.h>
// it is necessary to include glad before glfw
#include <GLFW/glfw3.h>

#include <algorithm>
#include <iostream>

void
_glCheckError(const char* file, int line, const char* comment);

#ifndef NDEBUG
#define check_error_gl(x) _glCheckError(__FILE__, __LINE__, x)
#else
#define check_error_gl(x) ((void)0)
#endif

#ifndef NDEBUG
#define ASSERT(condition, message)                                                                           \
    do {                                                                                                     \
        if (!(condition)) {                                                                                  \
            std::cerr << "Assertion `" #condition "` failed in " << __FILE__ << " line " << __LINE__ << ": " \
                      << message << std::endl;                                                               \
            std::terminate();                                                                                \
        }                                                                                                    \
    } while (false)
#define WARN(condition, message)                                                                             \
    do {                                                                                                     \
        if (!(condition)) {                                                                                  \
            std::cerr << "Assertion `" #condition "` failed in " << __FILE__ << " line " << __LINE__ << ": " \
                      << message << std::endl;                                                               \
        }                                                                                                    \
    } while (false)
#else
#define ASSERT(condition, message) \
    do {                           \
    } while (false)
#define WARN(condition, message) \
    do {                         \
    } while (false)
#endif
