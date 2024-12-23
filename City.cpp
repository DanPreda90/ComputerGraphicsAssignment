#pragma once
#include "City.h"
#include "Utils.h"
#include <iostream>
#include <random>

const glm::vec3 wave500(0.0f, 255.0f, 146.0f);
const glm::vec3 wave600(255.0f, 190.0f, 0.0f);
const glm::vec3 wave700(205.0f, 0.0f, 0.0f);
static glm::vec3 lightIntensity = 0.005f * (8.0f * wave500 + 15.6f * wave600 + 18.4f * wave700);

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

	glEnableVertexAttribArray(2);
	glGenBuffers(1, &b.uvBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, b.uvBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(b.uv_buffer_data), b.uv_buffer_data, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(3);
	glGenBuffers(1, &b.normalBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, b.normalBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(b.normal_buffer_data), b.normal_buffer_data, GL_STATIC_DRAW);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
	glGenBuffers(1, &b.indexBufferID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, b.indexBufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(b.index_buffer_data), b.index_buffer_data, GL_STATIC_DRAW);
	glBindVertexArray(0);
}

std::vector<glm::vec3> generateBuildingPositions(glm::vec3 origin,float spacing,int num_rings) {
	
	std::vector<glm::vec3> res;
	res.push_back(origin);
	for (int i = 1; i < num_rings; ++i) {
		float theta = 0.0f;
		int num_buildings = i * 6;
		float step = (2 * PI) / num_buildings;
		for (int j = 0; j < num_buildings; j++) {
			glm::vec3 pos = origin;
			pos.x = origin.x + (spacing * i) * cos(theta);
			pos.z = origin.z + (spacing * i) * sin(theta);
			res.push_back(pos);
			theta += step;
		}
	}
	return res;
}

void initializeCity(City & city,int num_buildings) {
	// Create and compile our GLSL program from the shaders
	city.programID = LoadShadersFromString(city.vertex_shader, city.fragment_shader);
	if (!city.programID) { return; }
	city.mvpMatrixID = glGetUniformLocation(city.programID, "MVP");
	city.lightIntensityID = glGetUniformLocation(city.programID, "lightIntensity");
	city.shadowMVPID = glGetUniformLocation(city.programID, "shadowMVP");
	city.shadowMapSamplerID = glGetUniformLocation(city.programID, "shadowMap");
	glm::vec3 position = glm::vec3(city.position);
	std::vector<glm::vec3> positions = generateBuildingPositions(city.position, city.spacing, num_buildings);
	for (int i = 0; i < positions.size(); ++i) {
		Building b;
		CalculateSurfaceNormals(b.normal_buffer_data, ArrayCount(b.vertex_buffer_data), b.vertex_buffer_data);
		b.position = positions[i];
		b.scale = city.scale;
		bindBuilding(b);
		// Get a handle for our "MVP" uniform
		city.buildings.push_back(b);
	}
	GLenum params[] = { GL_LINEAR,GL_LINEAR_MIPMAP_LINEAR,GL_REPEAT,GL_REPEAT };
	city.textureID = loadTexture(params, city.texture_file_path);
}

void renderCity(City& city, glm::mat4 vp,glm::mat4 shadowMVP,Light & light) {
	glUseProgram(city.programID);
	for (Building& b : city.buildings) {
		glm::mat4 modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::scale(modelMatrix, b.scale);
		modelMatrix = glm::translate(modelMatrix, b.position);
		glm::mat4 mvp = vp * modelMatrix;
		glm::mat4 shadow = shadowMVP * modelMatrix;
		glActiveTexture(GL_TEXTURE0);
		glUniform3fv(city.lightIntensityID, 1, &lightIntensity[0]);
		glUniform1f(city.textureSamplerID, 0);
		glBindTexture(GL_TEXTURE_2D, city.textureID);
		glActiveTexture(GL_TEXTURE1);
		glUniform1f(city.shadowMapSamplerID, 0);
		glBindTexture(GL_TEXTURE_2D, light.depthTextureID);
		glUniformMatrix4fv(city.shadowMVPID, 1, GL_FALSE, &shadow[0][0]);
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

void renderCityToShadow(City& city, Light& light) {
	glUseProgram(light.programID);
	glBindFramebuffer(GL_FRAMEBUFFER, light.frameBufferID);
	glClear(GL_DEPTH_BUFFER_BIT);
	for (Building & b : city.buildings) {
		glm::mat4 m = glm::scale(glm::mat4(1.0f), b.scale);
		m = glm::translate(m, b.position);
		//renderToShadowMap(light, b.VAO, m, 36);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}