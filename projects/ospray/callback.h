//!
//! This file defines some global variables and all callback functions
//!
#pragma once
#ifndef _CALLBACK_H_
#define _CALLBACK_H_

#include "common.h"

void Clean();

void error_callback(int error, const char *description);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

void cursor_position_callback(GLFWwindow *window, double xpos, double ypos);

void window_size_callback(GLFWwindow *window, int width, int height);

void render();

GLFWwindow *InitWindow();

void ShutdownWindow(GLFWwindow *window);

#endif//_CALLBACK_H_
