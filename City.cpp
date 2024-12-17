#pragma once
#include "City.h"
#include "Utils.h"
#include <iostream>
#include <random>

void bindBuilding(Building & b) {
	// Define scale of the building geometry
	// Create a vertex array object
	glGenVertexArrays(1, &b.VAO);
	glBindVertexArray(b.VAO);
	glEnableVertexAttribArray(0);
	// Create a vertex buffer object to store the vertex data		
	glGenBuffers(1, &b.vertexBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, b.vertexBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(b.vertex_buffer_data), b.vertex_buffer_data, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Create a vertex buffer object to store the color data
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &b.colorBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, b.colorBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(b.color_buffer_data), b.color_buffer_data, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	//glEnableVertexAttribArray(2);
	//glGenBuffers(1, &b.uvBufferID);
	//glBindBuffer(GL_ARRAY_BUFFER, b.uvBufferID);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(b.uv_buffer_data), b.uv_buffer_data.data(), GL_STATIC_DRAW);
	//glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	
	glGenBuffers(1, &b.indexBufferID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, b.indexBufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(b.index_buffer_data), b.index_buffer_data, GL_STATIC_DRAW);
	glBindVertexArray(0);
}

std::vector<glm::vec3> generateBuildingPositions(glm::vec3 origin,float spacing,int num_positions) {
	std::vector<glm::vec3> res;
	std::random_device d;
	std::mt19937 rng(d());
	std::uniform_real_distribution<> dis(0.5, 2 * 3.14159265359);
	glm::vec3 pos = origin;
	for (int i = 0; i < num_positions; ++i) {
		float f = dis(rng);
		pos.x = pos.x + spacing * cos(f);
		pos.z = pos.z + spacing * sin(f);
		res.push_back(pos);
	}
	return res;
}

void initializeCity(City & city,int num_buildings) {
	// Create and compile our GLSL program from the shaders
	city.programID = LoadShadersFromString(city.vertex_shader, city.fragment_shader);
	if (!city.programID) { return; }
	city.mvpMatrixID = glGetUniformLocation(city.programID, "MVP");
	glm::vec3 position = glm::vec3(city.position);
	std::vector<glm::vec3> positions = generateBuildingPositions(city.position, city.spacing, num_buildings);
	for (int i = 0; i < num_buildings; ++i) {
		Building b;
		b.position = positions[i];
		b.scale = city.scale;
		bindBuilding(b);
		// Get a handle for our "MVP" uniform
		
		//GLenum params[] = { GL_LINEAR,GL_LINEAR_MIPMAP_LINEAR,GL_REPEAT,GL_REPEAT };
		//city.textureID = loadTexture(params, city.texture_file_path);
		city.buildings.push_back(b);
	}
}

void renderCity(City& city, glm::mat4 vp) {
	glUseProgram(city.programID);
	for (Building& b : city.buildings) {
		glm::mat4 modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::scale(modelMatrix, b.scale);
		modelMatrix = glm::translate(modelMatrix, b.position);
		glm::mat4 mvp = vp * modelMatrix;
		glUniformMatrix4fv(city.mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);
		glBindVertexArray(b.VAO);
		glDrawElements(
			GL_TRIANGLES,     
			36,    			  
			GL_UNSIGNED_INT,  
			(void*)0          
		);
		glBindVertexArray(0);
	}
	glUseProgram(0);
}