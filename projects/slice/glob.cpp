#include "glob.hpp"
#include <cstdio>
#include <cmath>
#include <fstream>
#include <algorithm>
#ifdef USE_GLM
# include <glm/glm.hpp>
# include <glm/gtc/matrix_transform.hpp>
# include <glm/gtc/type_ptr.hpp>
#else
# error "GLM is required here"
#endif

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
struct Camera {
  size_t width  = 640, height = 480;
  float  aspect = (float) width / height;
  float  zNear = 1.f, zFar = 50.f;
  float  fovy = 30.f;
  glm::vec3 eye   = glm::vec3(0.f, 0.f, -5.f);
  glm::vec3 focus = glm::vec3(0.f);
  glm::vec3 up    = glm::vec3(0.f,1.f,0.f);
  glm::mat4 view, proj;
  glm::mat4 mvp; // cache
  void UpdateView() { view = glm::lookAt(eye, focus, up); }
  void UpdateProj() { proj = glm::perspective(fovy/180.f*(float)M_PI, aspect, zNear, zFar); }
  Camera() { UpdateView(); UpdateProj(); }
};
static Camera camera;

const glm::mat4& GetMVPMatrix()
{
  const float angle = (float) glfwGetTime() / 10.f;
  const glm::mat4 m =
    glm::rotate(glm::mat4(1.f), angle, glm::vec3(0,1,0)) *
    glm::rotate(glm::mat4(1.f), angle, glm::vec3(0,0,1));
  camera.mvp = camera.proj * camera.view * m;
  return camera.mvp;
}

const GLfloat* GetMVPMatrixPtr()
{
  return glm::value_ptr(GetMVPMatrix());
}

static const GLfloat box_texcoord[] =
{
  1, 1, 1,
  0, 1, 1,
  0, 0, 1,
  1, 0, 1,
  1, 0, 0,
  1, 1, 0,
  0, 1, 0,
  0, 0, 0
};

struct Intersect {
  float angle;
  glm::vec3 position;
  glm::vec3 texcoord;
};

static glm::vec3  ixMeanPos = glm::vec3(0.f, 0.f, 0.f);
static glm::vec3  ixMeanTex = glm::vec3(0.f, 0.f, 0.f);
static std::vector<Intersect> ixPts;

void IntersectReset() {
  ixMeanPos = glm::vec3(0.f, 0.f, 0.f);
  ixMeanTex = glm::vec3(0.f, 0.f, 0.f);
  ixPts.clear();
};

void IntersectPlane(const GLfloat box[24], const glm::ivec2 segment, const float z)
{
  // std::cout << "[IntersectPlane]" << std::endl;
  glm::vec3 vA (box[3 * segment.x + 0],
		box[3 * segment.x + 1],
		box[3 * segment.x + 2]);
  glm::vec3 vB (box[3 * segment.y + 0],
		box[3 * segment.y + 1],
		box[3 * segment.y + 2]);
  glm::vec3 tA (box_texcoord[3 * segment.x + 0],
		box_texcoord[3 * segment.x + 1],
		box_texcoord[3 * segment.x + 2]);
  glm::vec3 tB (box_texcoord[3 * segment.y + 0],
		box_texcoord[3 * segment.y + 1],
		box_texcoord[3 * segment.y + 2]); 
  if ((vA.z <= z && vB.z >= z) || (vB.z <= z && vA.z >= z))
  {
    // get intersection
    if (std::abs(vB.z - vA.z) < 0.001f) { return; }
    const float ratio = (z - vA.z) / (vB.z - vA.z);
    const glm::vec3 x = ratio * (vB - vA) + vA;
    const glm::vec3 t = ratio * (tB - tA) + tA;
    ixPts.push_back({0.f, x, t});
    // get center
    ixMeanPos += (x - ixMeanPos) / static_cast<float>(ixPts.size());
    ixMeanTex += (t - ixMeanTex) / static_cast<float>(ixPts.size());
  }
}

bool IntersectSortFunc(const Intersect& a, const Intersect& b) {
  return (a.angle < b.angle);
}

void IntersectSort()
{
  if (ixPts.empty()) { return; }
  for (auto& p : ixPts) {
    const glm::vec3 v = glm::normalize(p.position - ixMeanPos);
    if (std::abs(v.y) < 0.001f) {
      p.angle = v.x > 0 ? 0.f : M_PI;
    }
    else {
      const float tangent = v.y / v.x;
      p.angle = v.x > 0 ? atan(tangent) : atan(tangent) + M_PI;
    }
  }
  std::sort(ixPts.begin(), ixPts.end(), IntersectSortFunc);
}


void IntersectFetch(std::vector<GLfloat>& pos, std::vector<GLfloat>& tex)
{
  if (ixPts.empty()) { return; }
  pos.clear();
  tex.clear();
  pos.push_back(ixMeanPos.x);
  pos.push_back(ixMeanPos.y);
  pos.push_back(ixMeanPos.z);
  tex.push_back(ixMeanTex.x);
  tex.push_back(ixMeanTex.y);
  tex.push_back(ixMeanTex.z);
  for (auto& p : ixPts) {
    pos.push_back(p.position.x);
    pos.push_back(p.position.y);
    pos.push_back(p.position.z);
    tex.push_back(p.texcoord.x);
    tex.push_back(p.texcoord.y);
    tex.push_back(p.texcoord.z);
  };
  pos.push_back(ixPts[0].position.x);
  pos.push_back(ixPts[0].position.y);
  pos.push_back(ixPts[0].position.z);
  tex.push_back(ixPts[0].texcoord.x);
  tex.push_back(ixPts[0].texcoord.y);
  tex.push_back(ixPts[0].texcoord.z);
  // std::cout << std::endl;
  // for (int i = 0; i < pos.size() / 3; ++i) {
  //   printf("(%f,%f,%f), (%f,%f,%f)\n",
  // 	   pos[3 * i + 0],
  // 	   pos[3 * i + 1],
  // 	   pos[3 * i + 2],
  // 	   tex[3 * i + 0],
  // 	   tex[3 * i + 1],
  // 	   tex[3 * i + 2]);
  // }
  // std::cout << std::endl;
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

static void window_size_callback(GLFWwindow* window, int width, int height)
{
  glViewport(0, 0, width, height);
  camera.aspect = width / (float) height;
  camera.UpdateProj();
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
  GLFWwindow* window = glfwCreateWindow(camera.width, camera.height,
					"Sliced Based Volume Renderer", NULL, NULL);
  if (!window) { glfwTerminate(); exit(EXIT_FAILURE); }
  // Callback
  glfwSetKeyCallback(window, key_callback);
  glfwSetWindowSizeCallback(window, window_size_callback);
  // Ready
  glfwMakeContextCurrent(window);
  gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
  glfwSwapInterval(1);
  // Setup OpenGL
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_3D);
  return window;
}

