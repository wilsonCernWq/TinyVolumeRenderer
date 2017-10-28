#include "glob.hpp"
#include "camera.hpp"
#include "texture_reader.hpp"
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>    // std::max

template<class T>
constexpr const T& clamp( const T& v, const T& lo, const T& hi )
{
  return std::min(hi, std::max(lo, v));
}

//---------------------------------------------------------------------------------------

static GLuint tex_tfn_opaque = -1;
struct ColorPoint {
  float p;
  float r, g, b;
  unsigned long GetHex() {
    return
      ((static_cast<int>(r) & 0xff) << 16) +
      ((static_cast<int>(g) & 0xff) << 8) +
      ((static_cast<int>(b) & 0xff));
  }
};
struct OpacityPoint {
  float p;
  float a;
};
static std::vector<ColorPoint>   tfn_c =
{
  {0.0f, 0,   0,   255},
  {0.5f, 0,   255, 0  },
  {1.0f, 255, 0,   0  }
};
static std::vector<OpacityPoint> tfn_o =
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

static float lerp(const float& l, const float& r,
		  const float& pl, const float& pr, const float& p)
{
  const float dl = std::abs(pr - pl) > 0.0001f ? (p - pl) / (pr - pl) : 0.f;
  const float dr = 1.f - dl;
  return l * dr + r * dl;
}

static void UpdateTFN(GLuint tex_tfn_volume)
{
  // TODO better transfer function
  std::vector<GLubyte> tfn_volume;
  std::vector<GLubyte> tfn_opaque;
  // interpolate trasnfer function
  const int tfn_w = 100;
  const int tfn_h = 256;
  int cc_idx = 0;
  int co_idx = 0;
  // interpolate volume texture
  for (int i = 0; i < tfn_h; ++i)
  {
    const float p = clamp(i / (float)(tfn_h-1), 0.0f, 1.0f);
    // color
    {
      if (p > tfn_c[cc_idx+1].p) { ++cc_idx; }
      const float pl = tfn_c[cc_idx  ].p;
      const float pr = tfn_c[cc_idx+1].p;
      const int il = cc_idx;
      const int ir = cc_idx+1;
      const float r = lerp(tfn_c[il].r, tfn_c[ir].r, pl, pr, p);
      const float g = lerp(tfn_c[il].g, tfn_c[ir].g, pl, pr, p);
      const float b = lerp(tfn_c[il].b, tfn_c[ir].b, pl, pr, p);
      tfn_volume.push_back(r);
      tfn_volume.push_back(g);
      tfn_volume.push_back(b);
    }
    // opacity
    {
      if (p > tfn_o[co_idx+1].p) { ++co_idx; }
      const float pl = tfn_o[co_idx  ].p;
      const float pr = tfn_o[co_idx+1].p;    
      const int il = co_idx;
      const int ir = co_idx+1;
      const float a = lerp(tfn_o[il].a, tfn_o[ir].a, pl, pr, p);
      tfn_volume.push_back(clamp(a, 0.f, 1.f) * 255.f);
    }
  }
  // interpolate opaque palette
  for (int j = tfn_w - 1; j >= 0 ; --j) {
    for (int i = 0; i < tfn_h; ++i) {
      const float& r = tfn_volume[4 * i + 0];
      const float& g = tfn_volume[4 * i + 1];
      const float& b = tfn_volume[4 * i + 2];
      const float& a = tfn_volume[4 * i + 3];
      if (j / (float) tfn_w * 255.f > a) {
	tfn_opaque.push_back(r);
	tfn_opaque.push_back(g);
	tfn_opaque.push_back(b);
	tfn_opaque.push_back(255);
      }
      else {
      	tfn_opaque.push_back(0.5f * (255.f + r));
      	tfn_opaque.push_back(0.5f * (255.f + g));
      	tfn_opaque.push_back(0.5f * (255.f + b));
      	tfn_opaque.push_back(255);
      }
    }
  }
  updateTFN_custom(tex_tfn_volume, tfn_volume.data(), tfn_h, 1);
  updateTFN_custom(tex_tfn_opaque, tfn_opaque.data(), tfn_h, tfn_w);
}

void RenderGUI(GLuint tex_tfn_volume)
{
  // initialization
  ImGui_ImplGlfwGL3_NewFrame();
  // render GUI
  // ImGui::ShowTestWindow();
  if (ImGui::Begin("1D Transfer Function"))
  {
    // data process
    UpdateTFN(tex_tfn_volume);
    // draw
    ImGui::Text("1D Transfer Function");    
    float canvas_x = ImGui::GetCursorScreenPos().x;
    float canvas_y = ImGui::GetCursorScreenPos().y;
    float canvas_avail_x = ImGui::GetContentRegionAvail().x;
    float canvas_avail_y = ImGui::GetContentRegionAvail().y;
    {
      const float margin = 10.f;
      const float width  = canvas_avail_x - 2.f * margin;
      const float height = 60.f;
      // draw preview texture
      ImGui::SetCursorScreenPos(ImVec2(canvas_x + margin, canvas_y));
      ImGui::Image(reinterpret_cast<void*>(tex_tfn_opaque), ImVec2(width, height));
      ImGui::SetCursorScreenPos(ImVec2(canvas_x, canvas_y + height + margin));
      canvas_y       += height + margin;
      canvas_avail_y -= height + margin;
      // draw control points
      ImDrawList *draw_list = ImGui::GetWindowDrawList();
      for (int i = 0; i < tfn_o.size(); ++i) {
	const ImVec2 pos(canvas_x + width  * tfn_o[i].p + margin,
			 canvas_y - height * tfn_o[i].a - margin);
	draw_list->AddCircleFilled(pos, 7, 0xFF565656);
	draw_list->AddCircleFilled(pos, 6, 0xFFD8D8D8);
	draw_list->AddCircleFilled(pos, 4, 0xFF051c33);
	ImGui::SetCursorScreenPos(ImVec2(pos.x - 7, pos.y- 7));
	ImGui::InvisibleButton(("Button-"+std::to_string(i)).c_str(), ImVec2(14,14));
	if (ImGui::IsItemActive())
	{
	  ImVec2 value = ImGui::GetMouseDragDelta(0);
	  ImVec2 delta = ImGui::GetIO().MouseDelta;	  
	  tfn_o[i].a -= delta.y/height;
	  if (i > 0 && i < tfn_o.size()-1) {
	    tfn_o[i].p += delta.x/width;
	  }
	}
	ImGui::SetCursorScreenPos(ImVec2(canvas_x, canvas_y));
      }
    }
  }
  ImGui::End();
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
  fprintf(stdout, "[shader] reading vertex shader file %s\n", vshader_fname);
  fprintf(stdout, "[shader] reading fragment shader file %s\n", fshader_fname);
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
  // GUI
  {
    // Initialize GUI
    ImGui_ImplGlfwGL3_Init(window, false);
    // Create GUI Objects
    tex_tfn_opaque = loadTFN_custom();
  }
  return window;
}

void ShutdownWindow(GLFWwindow* window)
{
  // Shutup GUI
  ImGui_ImplGlfwGL3_Shutdown();
  // Shutup window
  glfwDestroyWindow(window);
}
