#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/gl.h>
#include <vector>
#include "Utils.h"

struct Building{
	glm::vec3 position;
	glm::vec3 scale;

	GLuint vertexBufferID;
	GLuint colorBufferID;
	GLuint VAO;
	GLuint uvBufferID;
	GLuint normalBufferID;
	GLuint indexBufferID;

	GLfloat vertex_buffer_data[72] = {	// Vertex definition for a canonical box
		// Front face
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,

		// Back face 
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,

		// Left face
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,

		// Right face 
		1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,

		// Top face
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,

		// Bottom face
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f
	};

	GLfloat normal_buffer_data[72] = { 0 };
	GLfloat color_buffer_data[72] =
	{
		// Front, red
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,

		// Back, yellow
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,

		// Left, green
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,

		// Right, cyan
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,

		// Top, blue
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,

		// Bottom, magenta
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f
	};
	GLuint index_buffer_data[36] = {
		0, 1, 2,
		0, 2, 3,

		4, 5, 6,
		4, 6, 7,

		8, 9, 10,
		8, 10, 11,

		12, 13, 14,
		12, 14, 15,

		16, 17, 18,
		16, 18, 19,

		20, 21, 22,
		20, 22, 23
	};
	GLfloat uv_buffer_data [48] = 
	{
		// Front
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		// Back
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		// Left
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		// Right
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		// Top - we do not want texture the top
		0.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 0.0f,
		// Bottom - we do not want texture the bottom
		0.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 0.0f,
	
	};

	
};

struct City {
	glm::vec3 scale;
	glm::vec3 position;
	glm::vec3 lightPosition;
	float spacing;
	std::vector<Building> buildings;
	GLuint programID;
	GLuint mvpMatrixID;
	GLuint lightIntensityID;
	GLuint textureID;
	GLuint shadowMapSamplerID;
	GLuint textureSamplerID;
	GLuint lightPositionID;
	GLuint shadowMVPID;
	int num_indices;
	const char* texture_file_path;

	const char* vertex_shader = R"(
		#version 330 core
		
		// Input
		layout(location = 0) in vec3 vertexPosition;
		layout(location = 1) in vec3 vertexColor;
		layout(location = 2) in vec2 vertexUV;
		layout(location = 3) in vec3 vertexNormal;

		// Output data, to be interpolated for each fragment
		out vec3 color;
		out vec2 uv;
		out vec3 normal;
		out vec4 shadowPos;
		out vec3 worldPosition;

		// Matrix for vertex transformation
		uniform mat4 MVP;
		uniform mat4 depthMVP;

		void main() {
			// Transform vertex
			gl_Position =  MVP * vec4(vertexPosition, 1);
    
			// Pass vertex color to the fragment shader
			color = vertexColor;
			uv = vertexUV;
			normal = vertexNormal;
			worldPosition = vertexPosition;
			shadowPos = depthMVP * vec4(vertexPosition,1.0f);
		}
	)";

	const char* fragment_shader = R"(
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
		float r = distance(worldPosition,lightPosition);
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
};

void initializeCity(City& city,int num_buildings);
void renderCity(City& city, glm::mat4 vp,glm::mat4 shadowMVP,Light& light);
void renderCityToShadow(City& city, Light & light);