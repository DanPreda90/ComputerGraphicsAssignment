#pragma once
#include "Utils.h"

// Skinning 
struct SkinObject {
	// Transforms the geometry into the space of the respective joint
	std::vector<glm::mat4> inverseBindMatrices;

	// Transforms the geometry following the movement of the joints
	std::vector<glm::mat4> globalJointTransforms;

	// Combined transforms
	std::vector<glm::mat4> jointMatrices;
};

// Animation 
struct SamplerObject {
	std::vector<float> input;
	std::vector<glm::vec4> output;
	int interpolation;
};
struct ChannelObject {
	int sampler;
	std::string targetPath;
	int targetNode;
};
struct AnimationObject {
	std::vector<SamplerObject> samplers;	// Animation data
};


struct Plane {
	glm::vec3 position;
	glm::vec3 scale;
	const std::string vertexShader = R"(
	#version 330 core

	layout(location = 0) in vec3 POSITION;
	layout(location = 1) in vec3 NORMAL;
	layout(location = 2) in vec2 TEXCOORD_0;
	//layout(location = 3) in vec3 TANGENT;

	uniform mat4 MVP;
	out vec2 uv;
	out vec3 normal;
	out vec3 tangent;

	void main() {
		gl_Position =  MVP * vec4(POSITION, 1.0f);
		uv = TEXCOORD_0;
		normal = NORMAL;
		//tangent = TANGENT;
	}
	)";

	const std::string fragmentShader = R"(
	#version 330 core
    #define PI 3.1415926535897932384626433832795
	in vec2 uv;
	in vec3 normal;
	//in vec3 tangent;

	out vec4 finalColor;
	uniform sampler2D textureSampler;
	uniform vec3 lightIntensity;

	void main()
	{
		finalColor = texture(textureSampler,uv);
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
	std::vector<AnimationObject> animationObjects;
	std::vector<SkinObject> skinObjects;
	std::vector<glm::mat4> meshMatrices;

	tinygltf::Model model;
	glm::mat4 model_transform = glm::mat4(1.0f);
};

void initalizePlane(Plane& plane);
void update(Plane& plane, float time);
void renderPlane(Plane& plane, glm::mat4 vp);