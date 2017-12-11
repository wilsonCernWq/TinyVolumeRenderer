//
// Created by qwu on 12/7/17.
//
#include "callback.h"
#include "global.h"
#include "widget.h"

#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>

#ifndef USE_TFN_MODULE
# error "Missing TransferFunctionModule!!!"
#else
# include "widgets/TransferFunctionWidget.h"
#endif

#ifdef USE_TFN_MODULE
static std::shared_ptr<tfn::tfn_widget::TransferFunctionWidget> tfnWidget;
#endif

//-------------------------------------------------------------------------------------------------------------//

void error_callback(int error, const char *description) {
  fprintf(stderr, "Error: %s\n", description);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) { glfwSetWindowShouldClose(window, GLFW_TRUE); }
}

void cursor_position_callback(GLFWwindow *window, double xpos, double ypos) {
  if (!ImGui::GetIO().WantCaptureMouse) {
    int left_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    int right_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
    if (left_state == GLFW_PRESS) {
      camera.CameraDrag((float)xpos, (float)ypos);
      ClearOSPRay();
    } else {
      camera.CameraBeginDrag((float)xpos, (float)ypos);
    }
    if (right_state == GLFW_PRESS) {
      camera.CameraZoom((float)xpos, (float)ypos);
      ClearOSPRay();
    } else {
      camera.CameraBeginZoom((float)xpos, (float)ypos);
    }
  }
}

void window_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
  camera.CameraUpdateProj((size_t)width, (size_t)height);
  ResizeOSPRay(width, height);
}

//-------------------------------------------------------------------------------------------------------------//

void RenderWindow(GLFWwindow *window)
{
  // init GUI
  {
    // Initialize GUI
    ImGui_ImplGlfwGL3_Init(window, false);
#ifdef USE_TFN_MODULE
    tfnWidget = std::make_shared<tfn::tfn_widget::TransferFunctionWidget>
      ([]() { return 256; },
       [&](const std::vector<float> &c, const std::vector<float> &a) {
         std::vector<float> o(a.size() / 2);
         for (size_t i = 0; i < a.size() / 2; ++i) { o[i] = a[2 * i + 1]; }
         volume.GetTransferFunction().Update(c.data(), o.data(), c.size() / 3, 1, o.size(), 1);
         ClearOSPRay();
       });
#endif
    tfn::tfn_widget::InitUI();
  }
  // start ospray
  StartOSPRay();
  // render opengl
  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // render GUI
    {
      UploadOSPRay();
      ImGui_ImplGlfwGL3_NewFrame();
#ifdef USE_TFN_MODULE
      if (tfnWidget->drawUI()) { tfnWidget->render(); };
#endif
      tfn::tfn_widget::DrawUI();
      ImGui::Render();
    }
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  // stop ospray
  StopOSPRay();
  // shutdown GUI
  {
    ImGui_ImplGlfwGL3_Shutdown();
  }
  // shutdown opengl
  glfwDestroyWindow(window);
  glfwTerminate();
}

GLFWwindow *CreateWindow() {
  // Initialize GLFW
  glfwSetErrorCallback(error_callback);
  if (!glfwInit()) { exit(EXIT_FAILURE); }
  // Provide Window Hints
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  // Create Window
  GLFWwindow *window = glfwCreateWindow((int)camera.CameraWidth(),
                                        (int)camera.CameraHeight(),
                                        "OSPRay Volume Test Renderer",
                                        nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }
  // Callback
  glfwSetKeyCallback(window, key_callback);
  glfwSetWindowSizeCallback(window, window_size_callback);
  glfwSetCursorPosCallback(window, cursor_position_callback);
  // Ready
  glfwMakeContextCurrent(window);
  gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
  glfwSwapInterval(1);
  check_error_gl("Ready");
  // Setup OpenGL
  glEnable(GL_DEPTH_TEST);
  check_error_gl("Setup OpenGL Options");
  return window;
}
