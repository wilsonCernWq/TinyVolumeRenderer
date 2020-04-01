//===========================================================================//
//                                                                           //
// Copyright(c) 2018 Qi Wu (Wilson)                                          //
// University of California, Davis                                           //
// MIT Licensed                                                              //
//                                                                           //
//===========================================================================//

#include "viewer.hpp"
#include "camera.hpp"
#include "texture.hpp"

#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <string>
#include <vector>

void
CheckShaderCompilationLog(GLuint shader, const std::string& fname)
{
    GLint isCompiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE) {
        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
        // The maxLength includes the NULL character
        std::vector<GLchar> errorLog(maxLength);
        glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);
        // Provide the infolog in whatever manor you deem best.
        // Exit with failure.
        glDeleteShader(shader); // Don't leak the shader.
        // show the message
        std::cerr << "compilation error for shader: " << fname << std::endl << errorLog.data() << std::endl;
    }
}

template<class T>
constexpr const T&
clamp(const T& v, const T& lo, const T& hi)
{
    return std::min(hi, std::max(lo, v));
}
//---------------------------------------------------------------------------------------
static GLuint tex_tfn_changed = true;
static GLuint tex_tfn_opaque  = -1;
struct ColorPoint {
    float         p;
    float         r, g, b;
    unsigned long GetHex()
    {
        return (0xff << 24) + ((static_cast<unsigned char>(b) & 0xff) << 16) +
               ((static_cast<unsigned char>(g) & 0xff) << 8) + ((static_cast<unsigned char>(r) & 0xff));
    }
};
struct OpacityPoint {
    float p;
    float a;
};
static std::vector<ColorPoint>   tfn_c = { { 0.0f, 0, 0, 255 },
                                         { 0.3f, 0, 255, 255 },
                                         { 0.6f, 255, 255, 0 },
                                         { 1.0f, 255, 0, 0 } };
static std::vector<OpacityPoint> tfn_o = { { 0.00f, 0.00 },
                                           { 0.25f, 0.25 },
                                           { 0.50f, 0.50 },
                                           { 0.75f, 0.75 },
                                           { 1.00f, 1.00 } };

template<typename T>
static int
find_idx(const T& A, float p, int l = -1, int r = -1)
{
    l     = l == -1 ? 0 : l;
    r     = r == -1 ? A.size() - 1 : r;
    int m = (r + l) / 2;
    if (A[l].p > p) {
        return l;
    }
    else if (A[r].p <= p) {
        return r;
    }
    else if ((m == l) || (m == r)) {
        return m + 1;
    }
    else {
        if (A[m].p <= p) {
            return find_idx(A, p, m, r);
        }
        else {
            return find_idx(A, p, l, m);
        }
    }
}

static bool
CapturedByGUI()
{
    ImGuiIO& io = ImGui::GetIO();
    return (io.WantCaptureMouse);
}

static float
lerp(const float& l, const float& r, const float& pl, const float& pr, const float& p)
{
    const float dl = std::abs(pr - pl) > 0.0001f ? (p - pl) / (pr - pl) : 0.f;
    const float dr = 1.f - dl;
    return l * dr + r * dl;
}

static void
UpdateTFN(GLuint tex_tfn_volume)
{
    // interpolate trasnfer function
    const int tfn_w = 100;
    const int tfn_h = 256;
    // TODO better transfer function
    std::vector<GLubyte> tfn_volume(tfn_h * 4);
    std::vector<GLubyte> tfn_opaque(tfn_w * tfn_h * 4);
    // interpolate volume texture
#pragma omp parallel for
    for (int i = 0; i < tfn_h; ++i) {
        const float p = clamp(i / (float)(tfn_h - 1), 0.0f, 1.0f);
        // color
        {
            const int   ir        = find_idx(tfn_c, p);
            const int   il        = ir - 1;
            const float pr        = tfn_c[ir].p;
            const float pl        = tfn_c[il].p;
            const float r         = lerp(tfn_c[il].r, tfn_c[ir].r, pl, pr, p);
            const float g         = lerp(tfn_c[il].g, tfn_c[ir].g, pl, pr, p);
            const float b         = lerp(tfn_c[il].b, tfn_c[ir].b, pl, pr, p);
            tfn_volume[4 * i + 0] = r;
            tfn_volume[4 * i + 1] = g;
            tfn_volume[4 * i + 2] = b;
        }
        // opacity
        {
            const int   ir        = find_idx(tfn_o, p);
            const int   il        = ir - 1;
            const float pr        = tfn_o[ir].p;
            const float pl        = tfn_o[il].p;
            const float a         = lerp(tfn_o[il].a, tfn_o[ir].a, pl, pr, p);
            tfn_volume[4 * i + 3] = clamp(a, 0.f, 1.f) * 255.f;
        }
    }
    // interpolate opaque palette
#pragma omp parallel for collapse(2)
    for (int j = 0; j < tfn_w; ++j) {
        for (int i = 0; i < tfn_h; ++i) {
            const float& r = tfn_volume[4 * i + 0];
            const float& g = tfn_volume[4 * i + 1];
            const float& b = tfn_volume[4 * i + 2];
            const float& a = tfn_volume[4 * i + 3];
            if ((1.f - j / (float)tfn_w) * 255.f > a) {
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

static void
ShowFixedInfoOverlay(bool open)
{
    //--------------------------------
    static bool  opened = false;
    static float fps    = 0.0f;
    static int   frames = 0;
    static auto  start  = std::chrono::system_clock::now();
    if (!opened) {
        start  = std::chrono::system_clock::now();
        frames = 0;
    }
    std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() - start;
    ++frames;
    // dont update this too frequently
    if (frames % 10 == 0 || frames == 1)
        fps = frames / elapsed_seconds.count();
    opened = open;
    //--------------------------------
    const float DISTANCE         = 10.0f;
    static int  corner           = 0;
    ImVec2      window_pos       = ImVec2((corner & 1) ? ImGui::GetIO().DisplaySize.x - DISTANCE : DISTANCE,
                               (corner & 2) ? ImGui::GetIO().DisplaySize.y - DISTANCE : DISTANCE);
    ImVec2      window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    // Transparent background
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.3f));
    if (ImGui::Begin("Information", NULL,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
                       ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::Separator();
        ImGui::Text("FPS (Hz): %.f\n", fps);
        ImGui::Separator();
        ImGui::End();
    }
    ImGui::PopStyleColor();
}

static void
ShowTFNWidget(GLuint tex_tfn_volume)
{
    if (!ImGui::Begin("Transfer Function Widget")) {
        ImGui::End();
        return;
    }
    ImGui::Text("1D Transfer Function");
    // radio paremeters
    static bool delete_point = 0;
    ImGui::Separator();
    ImGui::Checkbox("Delete Point", &delete_point);
    // draw stuffs
    ImDrawList* draw_list      = ImGui::GetWindowDrawList();
    const float mouse_x        = ImGui::GetMousePos().x;
    const float mouse_y        = ImGui::GetMousePos().y;
    const float scroll_x       = ImGui::GetScrollX();
    const float scroll_y       = ImGui::GetScrollY();
    float       canvas_x       = ImGui::GetCursorScreenPos().x;
    float       canvas_y       = ImGui::GetCursorScreenPos().y;
    float       canvas_avail_x = ImGui::GetContentRegionAvail().x;
    float       canvas_avail_y = ImGui::GetContentRegionAvail().y;
    const float margin         = 10.f;
    const float width          = canvas_avail_x - 2.f * margin;
    const float height         = 60.f;
    const float color_len      = 9.f;
    const float opacity_len    = 7.f;
    // draw preview texture
    ImGui::SetCursorScreenPos(ImVec2(canvas_x + margin, canvas_y));
    ImGui::Image(reinterpret_cast<void*>(tex_tfn_opaque), ImVec2(width, height));
    canvas_y += height + margin;
    canvas_avail_y -= height + margin;
    // draw color control points
    {
        ImGui::SetCursorScreenPos(ImVec2(canvas_x, canvas_y));
        // draw circle background
        draw_list->AddRectFilled(ImVec2(canvas_x + margin, canvas_y - margin),
                                 ImVec2(canvas_x + margin + width, canvas_y - margin + 2.5 * color_len), 0xFF474646);
        // draw circles
        for (int i = 0; i < tfn_c.size(); ++i) {
            const ImVec2 pos(canvas_x + width * tfn_c[i].p + margin, canvas_y);
            ImGui::SetCursorScreenPos(ImVec2(pos.x - color_len, pos.y));
            ImGui::InvisibleButton(("square-" + std::to_string(i)).c_str(), ImVec2(2.f * color_len, 2.f * color_len));
            ImGui::SetCursorScreenPos(ImVec2(canvas_x, canvas_y));
            // white background
            draw_list->AddTriangleFilled(ImVec2(pos.x - 0.5f * color_len, pos.y),
                                         ImVec2(pos.x + 0.5f * color_len, pos.y), ImVec2(pos.x, pos.y - color_len),
                                         0xFFD8D8D8);
            draw_list->AddCircleFilled(ImVec2(pos.x, pos.y + 0.5f * color_len), color_len, 0xFFD8D8D8);
            // dark highlight
            draw_list->AddCircleFilled(ImVec2(pos.x, pos.y + 0.5f * color_len), 0.5f * color_len,
                                       ImGui::IsItemHovered() ? 0xFF051C33 : 0xFFBCBCBC);
            // setup interactions
            if (ImGui::IsItemClicked(1)) {
                if (delete_point) {
                    if (i > 0 && i < tfn_c.size() - 1) {
                        tfn_c.erase(tfn_c.begin() + i);
                        tex_tfn_changed = true;
                    }
                }
            }
            if (ImGui::IsItemActive()) {
                ImVec2 delta = ImGui::GetIO().MouseDelta;
                if (i > 0 && i < tfn_c.size() - 1) {
                    tfn_c[i].p += delta.x / width;
                    tfn_c[i].p = clamp(tfn_c[i].p, tfn_c[i - 1].p, tfn_c[i + 1].p);
                }
                tex_tfn_changed = true;
            }
            // draw picker
            ImGui::SetCursorScreenPos(ImVec2(pos.x - color_len, pos.y + 1.5f * color_len));
            ImVec4 picked_color = ImColor((int)tfn_c[i].r, (int)tfn_c[i].g, (int)tfn_c[i].b, 255);
            if (ImGui::ColorEdit4(("ColorPicker" + std::to_string(i)).c_str(), (float*)&picked_color,
                                  ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoInputs |
                                    ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreview |
                                    ImGuiColorEditFlags_NoOptions)) {
                tfn_c[i].r      = picked_color.x * 255.f;
                tfn_c[i].g      = picked_color.y * 255.f;
                tfn_c[i].b      = picked_color.z * 255.f;
                tex_tfn_changed = true;
            }
            ImGui::SetCursorScreenPos(ImVec2(canvas_x, canvas_y));
        }
    }
    // draw opacity control points
    {
        ImGui::SetCursorScreenPos(ImVec2(canvas_x, canvas_y));
        // draw circles
        for (int i = 0; i < tfn_o.size(); ++i) {
            const ImVec2 pos(canvas_x + width * tfn_o[i].p + margin, canvas_y - height * tfn_o[i].a - margin);
            ImGui::SetCursorScreenPos(ImVec2(pos.x - opacity_len, pos.y - opacity_len));
            ImGui::InvisibleButton(("button-" + std::to_string(i)).c_str(),
                                   ImVec2(2.f * opacity_len, 2.f * opacity_len));
            ImGui::SetCursorScreenPos(ImVec2(canvas_x, canvas_y));
            // dark bounding box
            draw_list->AddCircleFilled(pos, opacity_len, 0xFF565656);
            // white background
            draw_list->AddCircleFilled(pos, 0.8f * opacity_len, 0xFFD8D8D8);
            // highlight
            draw_list->AddCircleFilled(pos, 0.6f * opacity_len, ImGui::IsItemHovered() ? 0xFF051c33 : 0xFFD8D8D8);
            // setup interaction
            if (ImGui::IsItemClicked(1)) {
                if (delete_point) {
                    if (i > 0 && i < tfn_o.size() - 1) {
                        tfn_o.erase(tfn_o.begin() + i);
                        tex_tfn_changed = true;
                    }
                }
            }
            if (ImGui::IsItemActive()) {
                ImVec2 delta = ImGui::GetIO().MouseDelta;
                tfn_o[i].a -= delta.y / height;
                tfn_o[i].a = clamp(tfn_o[i].a, 0.0f, 1.0f);
                if (i > 0 && i < tfn_o.size() - 1) {
                    tfn_o[i].p += delta.x / width;
                    tfn_o[i].p = clamp(tfn_o[i].p, tfn_o[i - 1].p, tfn_o[i + 1].p);
                }
                tex_tfn_changed = true;
            }
        }
    }
    // draw background interaction
    ImGui::SetCursorScreenPos(ImVec2(canvas_x + margin, canvas_y - margin));
    ImGui::InvisibleButton("tfn_palette", ImVec2(width, 2.5 * color_len));
    if (ImGui::IsItemClicked(1) && !delete_point) {
        const float p  = clamp((mouse_x - canvas_x - margin - scroll_x) / (float)width, 0.f, 1.f);
        const int   ir = find_idx(tfn_c, p);
        const int   il = ir - 1;
        const float pr = tfn_c[ir].p;
        const float pl = tfn_c[il].p;
        const float r  = lerp(tfn_c[il].r, tfn_c[ir].r, pl, pr, p);
        const float g  = lerp(tfn_c[il].g, tfn_c[ir].g, pl, pr, p);
        const float b  = lerp(tfn_c[il].b, tfn_c[ir].b, pl, pr, p);
        tfn_c.insert(tfn_c.begin() + ir, { p, r, g, b });
        tex_tfn_changed = true;
        printf("[GUI] add opacity point at %f with value = (%f, %f, %f)\n", p, r, g, b);
    }
    ImGui::SetCursorScreenPos(ImVec2(canvas_x, canvas_y));
    // draw background interaction
    ImGui::SetCursorScreenPos(ImVec2(canvas_x + margin, canvas_y - height - margin));
    ImGui::InvisibleButton("tfn_palette", ImVec2(width, height));
    if (ImGui::IsItemClicked(1) && !delete_point) {
        const float x   = clamp((mouse_x - canvas_x - margin - scroll_x) / (float)width, 0.f, 1.f);
        const float y   = clamp(-(mouse_y - canvas_y + margin - scroll_y) / (float)height, 0.f, 1.f);
        const int   idx = find_idx(tfn_o, x);
        tfn_o.insert(tfn_o.begin() + idx, { x, y });
        tex_tfn_changed = true;
        printf("[GUI] add opacity point at %f with value = %f\n", x, y);
    }
    ImGui::SetCursorScreenPos(ImVec2(canvas_x, canvas_y));
    ImGui::End();
}

void
RenderGUI(GLuint tex_tfn_volume)
{
    // Update TFN
    if (tex_tfn_changed) {
        UpdateTFN(tex_tfn_volume);
        tex_tfn_changed = false;
    }
    // initialization
    ImGui_ImplGlfwGL3_NewFrame();
    // render GUI
    ShowFixedInfoOverlay(true);
    ShowTFNWidget(tex_tfn_volume);
    ImGui::Render();
}

//---------------------------------------------------------------------------------------

static const char*
read_file(const char* fname)
{
    std::ifstream   file(fname, std::ios::binary | std::ios::ate | std::ios::in);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    char* buffer = new char[size + 1];
    buffer[size] = '\0';
    if (!file.read(const_cast<char*>(buffer), size)) {
        fprintf(stderr, "Error: Cannot read file %s\n", fname);
        exit(-1);
    }
    return buffer;
}

GLuint
LoadProgram(const char* vshader_fname, const char* fshader_fname)
{
    fprintf(stdout, "[shader] reading vertex shader file %s\n", vshader_fname);
    fprintf(stdout, "[shader] reading fragment shader file %s\n", fshader_fname);

    GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
    {
        const char* vshader_text = read_file(vshader_fname);
        glShaderSource(vshader, 1, &vshader_text, NULL);
        glCompileShader(vshader);
        CheckShaderCompilationLog(vshader, vshader_fname); // check error
        check_error_gl("Compile Vertex Shaders");
    }
    GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
    {
        const char* fshader_text = read_file(fshader_fname);
        glShaderSource(fshader, 1, &fshader_text, NULL);
        glCompileShader(fshader);
        CheckShaderCompilationLog(fshader, fshader_fname); // check error
        check_error_gl("Compile Fragment Shaders");
    }
    GLuint program = glCreateProgram();
    if (glCreateProgram == 0)
        throw std::runtime_error("wrong program");
    glAttachShader(program, vshader);
    glAttachShader(program, fshader);
    check_error_gl("Compile Shaders: Attach");
    glLinkProgram(program);
    check_error_gl("Compile Shaders: Link");
    glUseProgram(program);
    check_error_gl("Compile Shaders: Final");
    return program;
}

//---------------------------------------------------------------------------------------

static void
error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void
key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

static void
cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (!CapturedByGUI()) {
        int left_state  = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
        int right_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
        if (left_state == GLFW_PRESS) {
            CameraDrag(xpos, ypos);
        }
        else {
            CameraBeginDrag(xpos, ypos);
        }
        if (right_state == GLFW_PRESS) {
            CameraZoom(xpos, ypos);
        }
        else {
            CameraBeginZoom(xpos, ypos);
        }
    }
}

static void
window_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    CameraUpdateProjection(width, height);
}

GLFWwindow*
InitWindow()
{
    // Initialize GLFW
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }
    // Provide Window Hint
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    // Create Window
    GLFWwindow* window = glfwCreateWindow(CameraWidth(), CameraHeight(), "Raycast Volume Renderer", NULL, NULL);
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
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);
    // Setup OpenGL
    glEnable(GL_DEPTH_TEST);
    // GUI
    {
        // Initialize GUI
        ImGui_ImplGlfwGL3_Init(window, false);
        // Create GUI Objects
        tex_tfn_opaque = loadTFN_custom();
    }
    return window;
}

void
ShutdownWindow(GLFWwindow* window)
{
    // Shutup GUI
    ImGui_ImplGlfwGL3_Shutdown();
    // Shutup window
    glfwDestroyWindow(window);
}
