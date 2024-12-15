#pragma once

#include "Utils.h"
#include <glm/glm.hpp>
#include <tiny_gltf.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct PrimitiveObject {
	GLuint vao;
	GLuint texture;
	std::map<int, GLuint> vbos;
};

struct Model {
	const std::string vertexShader = R"(
	#version 330 core

	layout(location = 0) in vec3 POSITION;
	layout(location = 1) in vec3 NORMAL;
	layout(location = 2) in vec2 TEXCOORD_0;
	layout(location = 3) in vec4 COLOR_0;

	uniform mat4 MVP;
	out vec2 uv;
	out vec3 normal;
	out vec4 color;

	void main() {
		gl_Position =  MVP * vec4(POSITION, 1.0f);
		uv = TEXCOORD_0;
		normal = NORMAL;
		color = COLOR_0;
	}
)";

	const std::string fragmentShader = R"(
	#version 330 core

	in vec2 uv;
	in vec3 normal;
	in vec4 color;

	out vec4 finalColor;
	uniform sampler2D textureSampler;

	void main()
	{
		vec4 tex = texture(textureSampler,uv);
		finalColor = color + tex;
	}
	)";

	const char* gtlf_file_path;
	const char* texture_file_path;

	GLuint mvpMatrixID;
	GLuint shader;
	GLuint textureSampler;
	GLuint minValID;
	GLuint maxValID;
	
	std::vector<std::vector<PrimitiveObject>> primitiveObjects;

	tinygltf::Model model;
	glm::mat4 model_transform = glm::scale(glm::mat4(1.0f), glm::vec3(0.5, 0.5, 0.5));
};

void initializeModel(Model & model);
void renderModel(Model& model,glm::mat4 vp);