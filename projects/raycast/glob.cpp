#include "glob.hpp"
#include "camera.hpp"
#include "texture_reader.hpp"
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>    // std::max


//---------------------------------------------------------------------------------------

static GLuint texture_tf_palette = -1;
struct ColorPoint {
  float pos;
  float r, g, b;
};
struct OpacityPoint {
  float pos;
  float a;
};
static std::vector<ColorPoint> tfn_color =
{
  {0.0f, 0,   0,   255},
  {0.5f, 0,   255, 0  },
  {1.0f, 255, 0,   0  }
};
static std::vector<OpacityPoint> tfn_opacity =
{
  {0.0f, 0.0},
  {0.5f, 0.5},
  {1.0f, 1.0}
};

static bool CapturedByGUI()
{
  ImGuiIO& io = ImGui::GetIO();
  return (io.WantCaptureMouse);
}

void RenderGUI(GLuint texture_tf)
{
  // TODO better transfer function
  std::vector<GLubyte> tfn;
  std::vector<GLubyte> tfn_opaque;
  // Interpolate trasnfer function
  const int tfn_opaque_width  = 100;
  const int tfn_opaque_height = 256;
  int currCIdx = 0;
  int currOIdx = 0;
  for (int i = 0; i < tfn_opaque_height; ++i)
  {
    const float pos = std::min(1.f, std::max(0.f, i / (float)(tfn_opaque_height-1)));
    // color
    {
      if (pos > tfn_color[currCIdx+1].pos) { ++currCIdx; }
      const float pos_l = tfn_color[currCIdx  ].pos;
      const float pos_r = tfn_color[currCIdx+1].pos;    
      const int idx_l = currCIdx;
      const int idx_r = currCIdx+1;    
      const float dl = (pos_r - pos_l) > 0 ? (pos - pos_l) / (pos_r - pos_l) : 0;
      const float dr = 1.f - dl;
      const float r = tfn_color[idx_l].r * dr + tfn_color[idx_r].r * dl;
      const float g = tfn_color[idx_l].g * dr + tfn_color[idx_r].g * dl;
      const float b = tfn_color[idx_l].b * dr + tfn_color[idx_r].b * dl;
      tfn.push_back(r);
      tfn.push_back(g);
      tfn.push_back(b);
      //printf("%f, %f, %f, ",r, g, b);
    }
    // opacity
    {
      if (pos > tfn_opacity[currOIdx+1].pos) { ++currOIdx; }
      const float pos_l = tfn_opacity[currOIdx  ].pos;
      const float pos_r = tfn_opacity[currOIdx+1].pos;    
      const int idx_l = currOIdx;
      const int idx_r = currOIdx+1;
      const float dl = (pos_r - pos_l) > 0 ? (pos - pos_l) / (pos_r - pos_l) : 0;
      const float dr = 1.f - dl;
      const float a = tfn_opacity[idx_l].a * dr + tfn_opacity[idx_r].a * dl;
      tfn.push_back(a * 255);
      //printf("%f\n",a);
    }
  }
  for (int j = tfn_opaque_width - 1; j >= 0 ; --j) {
    for (int i = 0; i < tfn_opaque_height; ++i) {
      const float& r = tfn[4 * i + 0];
      const float& g = tfn[4 * i + 1];
      const float& b = tfn[4 * i + 2];
      const float& a = tfn[4 * i + 3];
      if (j / (float) tfn_opaque_width * 255 > a) {
	tfn_opaque.push_back(r);
	tfn_opaque.push_back(g);
	tfn_opaque.push_back(b);
	tfn_opaque.push_back(255);
      }
      else {
      	tfn_opaque.push_back(0.5 * (255 + r));
      	tfn_opaque.push_back(0.5 * (255 + g));
      	tfn_opaque.push_back(0.5 * (255 + b));
      	tfn_opaque.push_back(255);
      }
    }
  }
  updateTFN_custom(texture_tf, tfn.data(), tfn.size()/4, 1);
  updateTFN_custom(texture_tf_palette, tfn_opaque.data(),
		   tfn_opaque_height, tfn_opaque_width);
  
  // render GUI
  ImGui_ImplGlfwGL3_NewFrame();
  {
    ImGui::Text("1D Transfer Function");    
    float canvas_x = ImGui::GetCursorScreenPos().x;
    float canvas_y = ImGui::GetCursorScreenPos().y;
    float canvas_avail_x = ImGui::GetContentRegionAvail().x;
    float canvas_avail_y = ImGui::GetContentRegionAvail().y;
    {
      const float margin = 5.f;
      const float height = 60.f;
      canvas_x       += margin;
      ImGui::Image(reinterpret_cast<void*>(texture_tf_palette),
		   ImVec2(canvas_avail_x - margin, height));
      canvas_x       -= margin;
      canvas_y       += height + margin;
      canvas_avail_y -= height + margin;
      ImDrawList *draw_list = ImGui::GetWindowDrawList();
    }    
  }
  ImGui::Render();
}

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
					"Raycast Volume Renderer", NULL, NULL);
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
  texture_tf_palette = loadTFN_custom();
  return window;
}

void ShutdownWindow(GLFWwindow* window)
{
  // Shutup GUI
  ImGui_ImplGlfwGL3_Shutdown();
  // Shutup window
  glfwDestroyWindow(window);
}
