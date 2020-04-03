//===========================================================================//
//                                                                           //
// Copyright(c) 2018 Qi Wu (Wilson)                                          //
// University of California, Davis                                           //
// MIT Licensed                                                              //
//                                                                           //
//===========================================================================//

#include "opengl.hpp"

#include <vector>
#include <fstream>

//---------------------------------------------------------------------------------------
// error check helper from EPFL ICG class
static inline const char*
ErrorString(GLenum error)
{
    const char* msg;
    switch (error) {
#define Case(Token) \
    case Token: msg = #Token; break;
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

void
_glCheckError(const char* file, int line, const char* comment)
{
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        fprintf(stderr, "ERROR: %s (file %s, line %i: %s).\n", comment, file, line, ErrorString(error));
    }
}

//---------------------------------------------------------------------------------------

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

static const char*
ReadShaderFile(const char* fname)
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
        const char* vshader_text = ReadShaderFile(vshader_fname);
        glShaderSource(vshader, 1, &vshader_text, NULL);
        glCompileShader(vshader);
        CheckShaderCompilationLog(vshader, vshader_fname); // check error
        check_error_gl("Compile Vertex Shaders");
    }
    GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
    {
        const char* fshader_text = ReadShaderFile(fshader_fname);
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