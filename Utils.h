#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <tiny_gltf.h>
#include <string>

#define BUFFER_OFFSET(i) ((uint8_t *)0 + i)
#define PI 3.1415926535

struct PrimitiveObject {
	GLuint vao;
	GLuint texture;
	std::map<int, GLuint> vbos;
	int num_indices;
	int mesh_index;
};

struct Light {
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 intensity;
	GLuint programID;
	GLuint frameBufferID;
	GLuint shadowMapID;
	GLuint depthMatrixID;
	GLuint depthTextureID;
	GLuint depthTextureSamplerID;

	const char* depth_vertex_shader = R"(
		#version 330 core
		layout (location = 0) in vec3 aPos;

		uniform mat4 depthMVP;

		void main()
		{
			gl_Position = depthMVP * vec4(aPos, 1.0);
		}
	)";

	const char* depth_fragment_shader = R"(
		#version 330 core
		// Ouput data
		layout(location = 0) out float fragmentdepth;

		void main(){
			// Not really needed, OpenGL does it anyway
			fragmentdepth = gl_FragCoord.z;
		}
	)";

};

#define ArrayCount(arr) sizeof(arr)/sizeof(arr[0])

GLuint loadTexture(GLenum* params, const char* img_path);

GLuint LoadShadersFromFile(const char* vertex_file_path, const char* fragment_file_path);

GLuint LoadShadersFromString(std::string VertexShaderCode, std::string FragmentShaderCode);

void bindMesh(std::vector<PrimitiveObject>& primitiveObjects, tinygltf::Model& model, const char* texture_file_path, tinygltf::Mesh& mesh);
bool loadModel(tinygltf::Model& model, const char* filename);
void getSceneTransform(tinygltf::Model& model, glm::mat4& transform, tinygltf::Node& node);
void drawMesh(const std::vector<PrimitiveObject>& primitiveObjects, tinygltf::Model& model, GLuint textureSampler, tinygltf::Mesh& mesh);
void CalculateSurfaceNormals(GLfloat* out, size_t length, GLfloat* vertices);
void renderToShadowMap(Light& l, GLuint & vao, glm::mat4 depthMatrix, int indices);
void initializeFrameBuffer(Light& l);
void saveDepthTexture(GLuint fbo, std::string filename);