#pragma once
#include "Utils.h"
struct Quad {
	glm::vec3 position;
	glm::vec3 scale;
	GLfloat vertex_buffer_data[12] = {
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f
	};

	GLuint index_buffer_data[6] = {
		0, 1, 2,
		0, 2, 3
	};

	GLfloat color_buffer_data[12] = {
		0.0f,0.0f,1.0f,
		0.0f,0.0f,1.0f,
		0.0f,0.0f,1.0f,
		0.0f,0.0f,1.0f
	};

	GLuint programID;
	GLuint vao;
	GLuint vertexBufferID;
	GLuint colorBufferID;
	GLuint indexBufferID;
	GLuint MVPID;

	const char* vertex_shader = R"(
		#version 330 core
		
		// Input
		layout(location = 0) in vec3 vertexPosition;
		layout(location = 1) in vec3 vertexColor;

		// Output data, to be interpolated for each fragment
		out vec3 color;

		// Matrix for vertex transformation
		uniform mat4 MVP;

		void main() {
			// Transform vertex
			gl_Position =  MVP * vec4(vertexPosition, 1);
			// Pass vertex color to the fragment shader
			color = vertexColor;
		}
	)";

	const char* fragment_shader = R"(
	#version 330 core
	in vec3 color;
	out vec4 finalColor;
	void main()
	{
		finalColor = vec4(color,1.0f);
	}
	)";

};

void initQuad(Quad& quad);
void renderQuad(Quad& quad, glm::mat4 vp);