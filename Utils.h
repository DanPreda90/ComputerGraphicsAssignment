#pragma once
#include <glad/gl.h>
#include <string>


GLuint loadTexture(GLenum* params, const char* img_path);

GLuint LoadShadersFromFile(const char* vertex_file_path, const char* fragment_file_path);

GLuint LoadShadersFromString(std::string VertexShaderCode, std::string FragmentShaderCode);
