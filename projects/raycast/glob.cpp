#include "glob.hpp"
#include "camera.hpp"
#include "texture_reader.hpp"
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <chrono>

template<class T>
constexpr const T& clamp( const T& v, const T& lo, const T& hi )
{
  return std::min(hi, std::max(lo, v));
}
//---------------------------------------------------------------------------------------
static GLuint tex_tfn_changed = true;
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
  {0.00f, 0.00},
  {0.25f, 0.25},
  {0.50f, 0.50},
  {0.75f, 0.75},
  {1.00f, 1.00}
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
  // interpolate trasnfer function
  const int tfn_w = 100;
  const int tfn_h = 256;
  int cc_idx = 0;
  int co_idx = 0;
  // TODO better transfer function
  std::vector<GLubyte> tfn_volume(tfn_h * 4);
  std::vector<GLubyte> tfn_opaque(tfn_w * tfn_h * 4);
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
      tfn_volume[4 * i + 0] = r;
      tfn_volume[4 * i + 1] = g;
      tfn_volume[4 * i + 2] = b;
    }
    // opacity
    {
      if (p > tfn_o[co_idx+1].p) { ++co_idx; }
      const float pl = tfn_o[co_idx  ].p;
      const float pr = tfn_o[co_idx+1].p;    
      const int il = co_idx;
      const int ir = co_idx+1;
      const float a = lerp(tfn_o[il].a, tfn_o[ir].a, pl, pr, p);
      tfn_volume[4 * i + 3] = clamp(a, 0.f, 1.f) * 255.f;
    }
  }
  // interpolate opaque palette
# pragma omp parallel for collapse(2)
  for (int j = 0; j < tfn_w; ++j) {
    for (int i = 0; i < tfn_h; ++i) {
      const float& r = tfn_volume[4 * i + 0];
      const float& g = tfn_volume[4 * i + 1];
      const float& b = tfn_volume[4 * i + 2];
      const float& a = tfn_volume[4 * i + 3];
      if ((1.f - j / (float) tfn_w) * 255.f > a) {
	tfn_opaque[4 * (i + j * tfn_h) + 0] = r;
	tfn_opaque[4 * (i + j * tfn_h) + 1] = g;
	tfn_opaque[4 * (i + j * tfn_h) + 2] = b;
	tfn_opaque[4 * (i + j * tfn_h) + 3] = 255;
      }
      else {
      	tfn_opaque[4 * (i + j * tfn_h) + 0] = (0.5f * (255.f + r));
      	tfn_opaque[4 * (i + j * tfn_h) + 1] = (0.5f * (255.f + g));
      	tfn_opaque[4 * (i + j * tfn_h) + 2] = (0.5f * (255.f + b));
      	tfn_opaque[4 * (i + j * tfn_h) + 3] = (255);
      }
    }
  }
  updateTFN_custom(tex_tfn_volume, tfn_volume.data(), tfn_h, 1);
  updateTFN_custom(tex_tfn_opaque, tfn_opaque.data(), tfn_h, tfn_w);
}

static void ShowExampleAppFixedOverlay(bool open)
{
  //--------------------------------
  static bool opened = false;
  static float fps = 0.0f;
  static int frames = 0;
  static auto start = std::chrono::system_clock::now();
  if (!opened) {
    start = std::chrono::system_clock::now();
    frames = 0;
  }
  std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() - start;
  ++frames;
  // dont update this too frequently
  if (frames % 10 == 0 || frames == 1) fps = frames / elapsed_seconds.count();
  opened = open;
  //--------------------------------
  const float DISTANCE = 10.0f;
  static int corner = 0;
  ImVec2 window_pos = ImVec2((corner & 1) ?
			     ImGui::GetIO().DisplaySize.x - DISTANCE : DISTANCE,
			     (corner & 2) ?
			     ImGui::GetIO().DisplaySize.y - DISTANCE : DISTANCE);
  ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
  ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
  // Transparent background
  ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.3f));
  if (ImGui::Begin("Information", &open,
		   ImGuiWindowFlags_NoTitleBar|
		   ImGuiWindowFlags_NoResize|
		   ImGuiWindowFlags_AlwaysAutoResize|
		   ImGuiWindowFlags_NoMove|
		   ImGuiWindowFlags_NoSavedSettings))
  {
    ImGui::Text("FPS (Hz): %.f\n", fps);
    // ImGui::Separator();
    // ImGui::Text("Mouse Position: (%.1f,%.1f)",
    // 		ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y);
    ImGui::End();
  }
  ImGui::PopStyleColor();
}

void RenderGUI(GLuint tex_tfn_volume)
{
  // initialization
  ImGui_ImplGlfwGL3_NewFrame();
  // render GUI
  ShowExampleAppFixedOverlay(true);
  if (ImGui::Begin("1D Transfer Function"))
  {
    // data process
    if (tex_tfn_changed) {
      UpdateTFN(tex_tfn_volume);
      tex_tfn_changed = false;
    }
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
      canvas_y       += height + margin;
      canvas_avail_y -= height + margin;
      // draw control points
      ImDrawList *draw_list = ImGui::GetWindowDrawList();
      for (int i = 0; i < tfn_o.size(); ++i)
      {
	const ImVec2 pos(canvas_x + width  * tfn_o[i].p + margin,
			 canvas_y - height * tfn_o[i].a - margin);		
	ImGui::SetCursorScreenPos(ImVec2(pos.x - 7, pos.y- 7));
	ImGui::InvisibleButton(("button-"+std::to_string(i)).c_str(), ImVec2(14,14));
	ImGui::SetCursorScreenPos(ImVec2(canvas_x, canvas_y));
	draw_list->AddCircleFilled(pos, 7, 0xFF565656);
	draw_list->AddCircleFilled(pos, 6, 0xFFD8D8D8);
	draw_list->AddCircleFilled(pos, 4, ImGui::IsItemHovered() ?
				   0xFF051c33 : 0xFFD8D8D8);
	if (ImGui::IsItemActive())
	{
	  ImVec2 delta = ImGui::GetIO().MouseDelta;	  
	  tfn_o[i].a -= delta.y/height;
	  tfn_o[i].a = clamp(tfn_o[i].a, 0.0f, 1.0f);
	  if (i > 0 && i < tfn_o.size()-1) {
	    tfn_o[i].p += delta.x/width;
	    tfn_o[i].p = clamp(tfn_o[i].p, tfn_o[i-1].p, tfn_o[i+1].p);
	  }
	  tex_tfn_changed = true;
	}
      }
      // draw background
      ImGui::SetCursorScreenPos(ImVec2(canvas_x + margin, canvas_y - height - margin));
      ImGui::InvisibleButton("tfn_palette", ImVec2(width,height));
      if (ImGui::IsItemClicked(1))
      {
	const float x =  (ImGui::GetMousePos().x - canvas_x - margin - ImGui::GetScrollX());
	const float y = -(ImGui::GetMousePos().y - canvas_y + margin - ImGui::GetScrollY());
	printf("clicked background %f, %f\n", x/width, y/height);
      }	      
      ImGui::SetCursorScreenPos(ImVec2(canvas_x, canvas_y));
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
