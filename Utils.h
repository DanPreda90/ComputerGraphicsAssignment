#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <tiny_gltf.h>
#include <string>

#define BUFFER_OFFSET(i) ((uint8_t *)0 + i)

struct PrimitiveObject {
	GLuint vao;
	GLuint texture;
	std::map<int, GLuint> vbos;
};

#define ArrayCount(arr) sizeof(arr)/sizeof(arr[0])

GLuint loadTexture(GLenum* params, const char* img_path);

GLuint LoadShadersFromFile(const char* vertex_file_path, const char* fragment_file_path);

GLuint LoadShadersFromString(std::string VertexShaderCode, std::string FragmentShaderCode);

void bindMesh(std::vector<PrimitiveObject>& primitiveObjects, tinygltf::Model& model, const char* texture_file_path, tinygltf::Mesh& mesh);
bool loadModel(tinygltf::Model& model, const char* filename);
void getSceneTransform(tinygltf::Model& model, glm::mat4& transform, tinygltf::Node& node);
void drawMesh(const std::vector<PrimitiveObject>& primitiveObjects, tinygltf::Model& model, GLuint textureSampler, tinygltf::Mesh& mesh);
