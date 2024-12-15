#pragma once
#include <glm/glm.hpp>
#include <glad/gl.h>

struct SkyBox {
	glm::vec3 position;		// Position of the box 
	glm::vec3 scale;		// Size of the box in each axis
	const char* texture_file_path;

	const char* vertex_shader = R"(
		#version 330 core

		// Input
		layout(location = 0) in vec3 vertexPosition;
		layout(location = 1) in vec3 vertexColor;
		layout(location = 2) in vec2 vertexUV;

		// Output data, to be interpolated for each fragment
		out vec3 color;

		out vec2 uv;

		// Matrix for vertex transformation
		uniform mat4 MVP;

		void main() {
			// Transform vertex
			gl_Position =  MVP * vec4(vertexPosition, 1);
    
			// Pass vertex color to the fragment shader
			color = vertexColor;
			uv = vertexUV;
		}
	)";

	const char* fragment_shader = R"(
		#version 330 core

		in vec3 color;
		in vec2 uv;

		uniform sampler2D skyboxSampler;

		out vec3 finalColor;

		void main()
		{
			finalColor = texture(skyboxSampler,uv).rgb;
		}
	)";

	GLfloat vertex_buffer_data[72] = {
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f
	};

	GLfloat color_buffer_data[72] = {
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
		1.0f, 1.0f, 1.0f,
	};

	GLuint index_buffer_data[36] = {		// 12 triangle faces of a box
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
		20, 22, 23,
	};

	GLfloat uv_buffer_data[48] = {

		//Back
		0.75f, 0.33f,
		0.75f, 0.66f,
		1.0f, 0.66f,
		1.0f, 0.33f,

		// Left
		0.5f,0.66f,
		0.75f,0.66f,
		0.75f,0.33f,
		0.5f,0.33f,

		//Right
		0.0f, 0.66f,
		0.25f, 0.66f,
		0.25f, 0.33f,
		0.0f, 0.33f,

		// Front
		0.5f, 0.66f,
		0.5f, 0.33f,
		0.25f, 0.33f,
		0.25f, 0.66f,

		// Top - we do not want texture the top
		0.5f, 0.0f,
		0.25f, 0.0f,
		0.25f, 0.33f,
		0.5f, 0.33f,

		// Bottom - we do not want texture the bottom
		0.5f, 1.0f,
		0.5f, 0.66f,
		0.25f, 0.66f,
		0.25f, 1.0f,
	};
	// OpenGL buffers
	GLuint vertexArrayID;
	GLuint vertexBufferID;
	GLuint indexBufferID;
	GLuint colorBufferID;
	GLuint uvBufferID;
	GLuint textureID;

	// Shader variable IDs
	GLuint mvpMatrixID;
	GLuint textureSamplerID;
	GLuint programID;
	GLuint skyboxSamplerID;
};

void initialize(SkyBox & sky);
void render(SkyBox & sky,glm::mat4 vp);
