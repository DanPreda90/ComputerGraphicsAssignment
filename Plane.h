#pragma once
#include "Utils.h"


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
	float rotation;

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
	in vec4 color;

	out vec4 finalColor;
	uniform sampler2D textureSampler;
	uniform sampler2D depthSampler;
	uniform vec3 lightIntensity;

	float shadowCalculation(vec4 shadowPos)
	{
			vec3 projection = shadowPos.xyz / shadowPos.w;
			projection = projection * 0.5 + 0.5;
			float lightDepth = texture(depthSampler,projection.xy).r;
			float cameraDepth = shadowPos.z;

			return (cameraDepth >= lightDepth + 1e-4) ? 0.8 : 1.0;
	}

	void main()
	{
		vec3 lightDirection = normalize(vec3(0,1,0));
		vec3 reflection = (0.78 / PI) * max(dot(normal,lightDirection),0) * lightIntensity;
		vec4 tex = texture(textureSampler,uv) * (0.15 * 0.85);
		vec4 rgb = vec4(reflection.xyz,0) * tex;
		vec4 tonemapped = rgb / (1 + rgb);
		vec3 ambient = 0.15 * lightIntensity;
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
	std::vector<AnimationObject> animationObjects;
	std::vector<glm::mat4> localNodeTransforms;
	std::vector<glm::mat4> meshMatrices;

	tinygltf::Model * model;
	glm::mat4 model_transform = glm::mat4(1.0f);
};

void initalizePlane(Plane& plane,tinygltf::Model * model);
void update(Plane& plane, float time);
void renderPlane(Plane& plane, glm::mat4 vp);
void renderPlaneShadow(Plane& plane, Light& light);