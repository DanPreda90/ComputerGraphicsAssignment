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
	out vec3 worldPosition;

	void main() {
		gl_Position =  MVP * vec4(POSITION, 1.0f);
		uv = TEXCOORD_0;
		normal = NORMAL;
		worldPosition = POSITION;
		//tangent = TANGENT;
	}
	)";

	const std::string fragmentShader = R"(
	#version 330 core
    #define PI 3.1415926535897932384626433832795
	in vec2 uv;
	in vec3 normal;
	in vec4 color;
	in vec3 worldPosition;

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
			float cameraDepth = shadowPos.z;

			return (cameraDepth > lightDepth) ? 0.2 : 1.0;
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
	GLuint depthMapSamplerID;
	GLuint lightPositionID;

	std::vector<std::vector<PrimitiveObject>> primitiveObjects;
	std::vector<AnimationObject> animationObjects;
	std::vector<glm::mat4> localNodeTransforms;
	std::vector<glm::mat4> meshMatrices;

	tinygltf::Model * model;
	glm::mat4 model_transform = glm::mat4(1.0f);
};

void initalizePlane(Plane& plane,tinygltf::Model * model);
void update(Plane& plane, float time);
void renderPlane(Plane& plane, glm::mat4 vp,Light & light);
void renderPlaneShadow(Plane& plane, Light& light);