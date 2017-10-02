#include "glob.hpp"
#include <cstdio>
#include <fstream>
#ifdef USE_GLM
# include <glm/glm.hpp>
# include <glm/gtc/matrix_transform.hpp>
# include <glm/gtc/type_ptr.hpp>
#else
# error "GLM is required here"
#endif

//---------------------------------------------------------------------------------------
// error check helper from EPFL ICG class
static inline const char* ErrorString(GLenum error) {
  const char* msg;
  switch (error) {
#define Case(Token)  case Token: msg = #Token; break;
    Case(GL_INVALID_ENUM);
    Case(GL_INVALID_VALUE);
    Case(GL_INVALID_OPERATION);
    Case(GL_INVALID_FRAMEBUFFER_OPERATION);
    Case(GL_NO_ERROR);
    Case(GL_OUT_OF_MEMORY);
#undef Case
  }
  return msg;
}

void _glCheckError(const char* file, int line, const char* comment) {
  GLenum error;
  while ((error = glGetError()) != GL_NO_ERROR) {
    fprintf(stderr, "ERROR: %s (file %s, line %i: %s).\n",
	    comment, file, line, ErrorString(error));
  }
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
  float  zNear = 1.f, zFar = 20.f;
  float  fovy = 30.f;
  glm::vec3 eye   = glm::vec3(-10.f, 0.f, 0.f);
  glm::vec3 focus = glm::vec3(0.f);
  glm::vec3 up    = glm::vec3(0.f,0.f,1.f);
  glm::mat4 view, proj;
  glm::mat4 mvp; // cache
  void UpdateView() { view = glm::lookAt(eye, focus, up); }
  void UpdateProj() { proj = glm::perspective(fovy/180.f*(float)M_PI, aspect, zNear, zFar); }
  Camera() { UpdateView(); UpdateProj(); }
};
static Camera camera;

const GLfloat* GetMVPMatrix()
{
  glm::mat4 m = glm::rotate(glm::mat4(1.f), (float) glfwGetTime(), glm::vec3(0,0,1));
  camera.mvp = camera.proj * camera.view * m;
  return glm::value_ptr(camera.mvp);
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
  return window;
}

