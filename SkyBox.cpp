#include "SkyBox.h"
#include "Utils.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>


void initialize(SkyBox & sky) {
	// Define scale of the building geometry
	// Create a vertex array object

	glGenVertexArrays(1, &sky.vertexArrayID);
	glBindVertexArray(sky.vertexArrayID);
	glEnableVertexAttribArray(0);
	// Create a vertex buffer object to store the vertex data		
	glGenBuffers(1, &sky.vertexBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, sky.vertexBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sky.vertex_buffer_data), sky.vertex_buffer_data, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Create a vertex buffer object to store the color data
	// TODO: 
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &sky.colorBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, sky.colorBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sky.color_buffer_data), sky.color_buffer_data, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(2);
	glGenBuffers(1, &sky.uvBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, sky.uvBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sky.uv_buffer_data), sky.uv_buffer_data, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

	// Create an index buffer object to store the index data that defines triangle faces
	glGenBuffers(1, &sky.indexBufferID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sky.indexBufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sky.index_buffer_data), sky.index_buffer_data, GL_STATIC_DRAW);
	glBindVertexArray(0);
	// Create and compile our GLSL program from the shaders
	sky.programID = LoadShadersFromString(sky.vertex_shader,sky.fragment_shader);
	if (sky.programID == 0)
	{
		std::cerr << "Failed to load shaders." << std::endl;
	}

	// Get a handle for our "MVP" uniform
	sky.mvpMatrixID = glGetUniformLocation(sky.programID, "MVP");

	GLenum params[] = { GL_LINEAR,GL_LINEAR_MIPMAP_LINEAR,GL_REPEAT,GL_REPEAT };
	sky.textureID = loadTexture(params,sky.texture_file_path);

	sky.skyboxSamplerID = glGetUniformLocation(sky.programID, "skyboxSampler");
}

void render(SkyBox & sky,glm::mat4 vp) {
	glDepthMask(GL_FALSE);
	glUseProgram(sky.programID);

	// TODO: Model transform 
	// -----------------------
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	// Scale the box along each axis to make it look like a building
	modelMatrix = glm::scale(modelMatrix, sky.scale);

	// -----------------------

	// Set model-view-projection matrix
	glm::mat4 mvp = vp * modelMatrix;
	glUniformMatrix4fv(sky.mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sky.textureID);
	glUniform1i(sky.skyboxSamplerID, 0);

	glBindVertexArray(sky.vertexArrayID);
	// Draw the box
	glDrawElements(
		GL_TRIANGLES,      // mode
		36,    			   // number of indices
		GL_UNSIGNED_INT,   // type
		(void*)0           // element array buffer offset
	);
	glBindVertexArray(0);
	glDepthMask(GL_TRUE);
}

void cleanup(SkyBox & sky) {
	glDeleteBuffers(1, &sky.vertexBufferID);
	glDeleteBuffers(1, &sky.colorBufferID);
	glDeleteBuffers(1, &sky.indexBufferID);
	glDeleteVertexArrays(1, &sky.vertexArrayID);
	glDeleteBuffers(1, &sky.uvBufferID);
	glDeleteTextures(1, &sky.textureID);
	glDeleteProgram(sky.programID);
}

	