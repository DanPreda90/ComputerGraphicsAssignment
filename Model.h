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
	uniform mat4 lightMVP;
	out vec2 uv;
	out vec3 normal;
	out vec4 color;
	out vec4 shadowPos;
	out vec3 worldPosition;

	void main() {
		gl_Position =  MVP * vec4(POSITION, 1.0f);
		shadowPos = lightMVP * vec4(POSITION,1.0f);
		uv = TEXCOORD_0;
		normal = NORMAL;
		color = COLOR_0;
		worldPosition = POSITION;
	}
	)";

	const std::string fragmentShader = R"(
	#version 330 core
    #define PI 3.1415926535897932384626433832795
	in vec2 uv;
	in vec3 normal;
	in vec4 color;
	in vec3 worldPosition;
	in vec4 shadowPos;

	out vec4 finalColor;
	uniform sampler2D textureSampler;
	uniform sampler2D depthSampler;
	uniform vec3 lightIntensity;
	uniform vec3 lightPosition;

	float shadowCalculation(vec4 shadowPos)
	{
			vec3 projection = shadowPos.xyz / shadowPos.w;
			projection = projection * 0.5 + 0.5;
			float lightDepth = texture(depthSampler,projection.xy).r;
			float cameraDepth = projection.z;

			return (cameraDepth > lightDepth + 1.0e-2) ? 1.0 : 0.2;
	}

	void main()
	{
		float r = distance(lightPosition,worldPosition);
		float irradiance = 4 * PI * r * r;
		vec3 lightDirection = normalize(lightPosition - worldPosition);
		vec3 reflection = (0.78 / PI) * max(dot(normal,lightDirection),0) * (lightIntensity / vec3(irradiance,irradiance,irradiance));
		vec4 tex = texture(textureSampler,uv) * (0.15 * 0.85);
		vec4 rgb = vec4(reflection.xyz,0) * tex;
		vec4 tonemapped = rgb / (1 + rgb);
		vec3 ambient = 0.15 * lightIntensity;
		finalColor = shadowCalculation(shadowPos) * pow(tonemapped,vec4(1.0/2.2));
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
	GLuint depthMapSamplerID;
	GLuint lightPositionID;
	GLuint lightMVPID;
	
	std::vector<std::vector<PrimitiveObject>> primitiveObjects;

	tinygltf::Model model;
	glm::mat4 model_transform = glm::scale(glm::mat4(1.0f), glm::vec3(0.5,0.5,0.5));
};

void initializeModel(Model & model);
void renderModel(Model& model,glm::mat4 vp, Light& light);
void renderModelShadow(Model& model, Light& light);
