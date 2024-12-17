#pragma once

#include "Utils.h"

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
    #define PI 3.1415926535897932384626433832795
	in vec2 uv;
	in vec3 normal;
	in vec4 color;

	out vec4 finalColor;
	uniform sampler2D textureSampler;
	uniform vec3 lightIntensity;

	void main()
	{
		vec3 lightDirection = vec3(0,1,0);
		vec3 reflection = (0.78 / PI) * max(dot(normal,lightDirection),0) * lightIntensity;
		vec4 tex = texture(textureSampler,uv) * (0.15 * 0.85);
		vec4 rgb = vec4(reflection.xyz,0) * tex;
		vec4 tonemapped = rgb / (1 + rgb);
		finalColor = pow(tonemapped,vec4(1.0/2.2));
	}
	)";

	const char* gtlf_file_path;
	const char* texture_file_path;

	GLuint mvpMatrixID;
	GLuint shader;
	GLuint textureSampler;
	GLuint minValID;
	GLuint maxValID;
	GLuint lightIntensityID;
	
	std::vector<std::vector<PrimitiveObject>> primitiveObjects;

	tinygltf::Model model;
	glm::mat4 model_transform = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
};

void initializeModel(Model & model);
void renderModel(Model& model,glm::mat4 vp);