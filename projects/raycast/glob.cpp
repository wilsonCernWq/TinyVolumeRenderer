#include "glob.hpp"
#include "camera.hpp"
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>
#include <fstream>

//---------------------------------------------------------------------------------------

static bool CapturedByGUI();

//---------------------------------------------------------------------------------------

static const char* read_file(const char* fname)
{
  std::ifstream file(fname, std::ios::binary | std::ios::ate);
  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);
  char* buffer = new char[size+1]; buffer[size] = '\0';
  if (!file.read(const_cast<char*>(buffer), size)) {
    fprintf(stderr, "Error: Cannot read file %s\n", fname);
    exit(-1);
  }
  return buffer;
}

GLuint LoadProgram(const char* vshader_fname, const char* fshader_fname)
{
  fprintf(stdout, "reading vertex shader file %s\n", vshader_fname);
  fprintf(stdout, "reading fragment shader file %s\n", fshader_fname);
  GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
  const char* vshader_text = read_file(vshader_fname);
  glShaderSource(vshader, 1, &vshader_text, NULL);
  glCompileShader(vshader);
  GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
  const char* fshader_text = read_file(fshader_fname);
  glShaderSource(fshader, 1, &fshader_text, NULL);
  glCompileShader(fshader);
  GLuint program = glCreateProgram();
  glAttachShader(program, vshader);
  glAttachShader(program, fshader);
  glLinkProgram(program);
  glUseProgram(program);
  check_error_gl("Compile Shaders");
  return program;
}

//---------------------------------------------------------------------------------------

static void error_callback(int error, const char* description)
{
  fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
  { glfwSetWindowShouldClose(window, GLFW_TRUE); }
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
  if (!CapturedByGUI())
  {
    int left_state  = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    int right_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
    if (left_state == GLFW_PRESS) { CameraDrag(xpos, ypos); }
    else { CameraBeginDrag(xpos, ypos); }
    if (right_state == GLFW_PRESS) { CameraZoom(xpos, ypos); }
    else { CameraBeginZoom(xpos, ypos); }
  }
}

static void window_size_callback(GLFWwindow* window, int width, int height)
{
  glViewport(0, 0, width, height);
  CameraUpdateProj(width, height);
}

GLFWwindow* InitWindow()
{
  // Initialize GLFW
  glfwSetErrorCallback(error_callback);
  if (!glfwInit()) { exit(EXIT_FAILURE); }
  // Provide Window Hnits
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
  // Create Window
  GLFWwindow* window = glfwCreateWindow(CameraWidth(), CameraHeight(),
					"Sliced Based Volume Renderer", NULL, NULL);
  if (!window) { glfwTerminate(); exit(EXIT_FAILURE); }
  // Callback
  glfwSetKeyCallback(window, key_callback);
  glfwSetWindowSizeCallback(window, window_size_callback);
  glfwSetCursorPosCallback(window, cursor_position_callback);
  // Ready
  glfwMakeContextCurrent(window);
  gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
  glfwSwapInterval(1);
  // Setup OpenGL
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_3D);
  // Initialize GUI
  ImGui_ImplGlfwGL3_Init(window, false);
  return window;
}

void ShutdownWindow(GLFWwindow* window)
{
  ImGui_ImplGlfwGL3_Shutdown();
  glfwDestroyWindow(window);
}

//---------------------------------------------------------------------------------------

static bool CapturedByGUI()
{
  ImGuiIO& io = ImGui::GetIO();
  return (io.WantCaptureMouse);
}

void RenderGUI()
{
  // render GUI
  ImGui_ImplGlfwGL3_NewFrame();
  {
    ImGui::Button("xx");
    ImGui::Text("Terrain Parameter");
    ImGui::Text("Control");
  }
  ImGui::Render();
}
