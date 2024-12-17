#pragma once
#include "Model.h"
#include <glad/gl.h>
#include <string>
#include <iostream>

const glm::vec3 wave500(0.0f, 255.0f, 146.0f);
const glm::vec3 wave600(255.0f, 190.0f, 0.0f);
const glm::vec3 wave700(205.0f, 0.0f, 0.0f);
static glm::vec3 lightIntensity = 0.005f * (8.0f * wave500 + 15.6f * wave600 + 18.4f * wave700);

void bindModelNodes(Model& m,
	tinygltf::Node& node) {
	// Bind buffers for the current mesh at the node
	if ((node.mesh >= 0) && (node.mesh < m.model.meshes.size())) {
		std::vector<PrimitiveObject> primitives;
		bindMesh(primitives, m.model, m.texture_file_path, m.model.meshes[node.mesh]);
		m.primitiveObjects.push_back(primitives);
	}

	// Recursive into children nodes
	for (size_t i = 0; i < node.children.size(); i++) {
		assert((node.children[i] >= 0) && (node.children[i] < m.model.nodes.size()));
		bindModelNodes(m, m.model.nodes[node.children[i]]);
	}
}

void bindModel(Model& m) {

	tinygltf::Scene& scene = m.model.scenes[m.model.defaultScene];
	for (int i = 0; i < scene.nodes.size(); ++i) {
		bindModelNodes(m, m.model.nodes[scene.nodes[i]]);
	}
}

void initializeModel(Model& model) {
	model.shader = LoadShadersFromString(model.vertexShader, model.fragmentShader);

	if (!model.shader) { std::cerr << "Failed to load shaders." << std::endl; }
	if (!loadModel(model.model, model.gtlf_file_path)) {
		std::cerr << "Failed to load model." << std::endl;
	}
	tinygltf::Scene scene = model.model.scenes[0];
	getSceneTransform(model.model, model.model_transform, model.model.nodes[scene.nodes[0]]);
	bindModel(model);
	model.lightIntensityID = glGetUniformLocation(model.shader, "lightIntensity");
	model.mvpMatrixID = glGetUniformLocation(model.shader, "MVP");
	model.textureSampler = glGetUniformLocation(model.shader, "textureSampler");
}

void drawModelNodes(Model& m,tinygltf::Node& node) {
	// Draw the mesh at the node, and recursively do so for children nodes
	if ((node.mesh >= 0) && (node.mesh < m.model.meshes.size())) {
		drawMesh(m.primitiveObjects[node.mesh], m.model, m.textureSampler,m.model.meshes[node.mesh]);
		return;
	}
	for (size_t i = 0; i < node.children.size(); i++) {
		drawModelNodes(m,m.model.nodes[node.children[i]]);
	}
}
void drawModel(Model & m) {
	// Draw all nodes
	const tinygltf::Scene& scene = m.model.scenes[m.model.defaultScene];
	for (size_t i = 0; i < scene.nodes.size(); ++i) {
		drawModelNodes(m,m.model.nodes[scene.nodes[i]]);
	}
}

void renderModel(Model & model,glm::mat4 vp) {
	glUseProgram(model.shader);
	glm::mat4 mvp = model.model_transform * vp;
	glUniform3fv(model.lightIntensityID,1,&lightIntensity[0]);
	glUniformMatrix4fv(model.mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);
	drawModel(model);
}