#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/gl.h>
#include <vector>


struct Building{
	glm::vec3 position;
	glm::vec3 scale;

	GLuint vertexBufferID;
	GLuint colorBufferID;
	GLuint VAO;
	GLuint uvBufferID;
	GLuint textureID;
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
	std::vector<GLfloat> uv_buffer_data;

	const char* texture_file_path;
};

struct City {
	glm::vec3 scale;
	glm::vec3 position;
	float spacing;
	std::vector<Building> buildings;
	GLuint programID;
	GLuint mvpMatrixID;
	GLuint textureSamplerID;

	const char* vertex_shader = R"(
		#version 330 core

		// Input
		layout(location = 0) in vec3 vertexPosition;
		layout(location = 1) in vec3 vertexColor;
		//layout(location = 2) in vec2 vertexUV;

		// Output data, to be interpolated for each fragment
		out vec3 color;
		//out vec2 uv;

		// Matrix for vertex transformation
		uniform mat4 MVP;

		void main() {
			// Transform vertex
			gl_Position =  MVP * vec4(vertexPosition, 1);
    
			// Pass vertex color to the fragment shader
			color = vertexColor;
			//uv = vertexUV;
		}
	)";

	const char* fragment_shader = R"(
		#version 330 core

		in vec3 color;
		//in vec2 uv;

		//uniform sampler2D skyboxSampler;

		out vec3 finalColor;

		void main()
		{
			finalColor = color;
		}
	)";
};

void initializeCity(City& city,int num_buildings);
void renderCity(City& city, glm::mat4 vp);