#pragma once
#include "Utils.h"
#include "Environment.h"
#include <glad/gl.h>
#include <string>
#include <iostream>

#define BUFFER_OFFSET(i) ((uint8_t *)0 + i)
void bindMesh(std::vector<PrimitiveObject>& primitiveObjects,
	tinygltf::Model& model, const char* texture_file_path, tinygltf::Mesh& mesh) {
	std::map<int, GLuint> vbos;
	for (size_t i = 0; i < model.bufferViews.size(); ++i) {
		const tinygltf::BufferView& bufferView = model.bufferViews[i];

		int target = bufferView.target;

		if (bufferView.target == 0) {
			// The bufferView with target == 0 in our model refers to 
			// the skinning weights, for 25 joints, each 4x4 matrix (16 floats), totaling to 400 floats or 1600 bytes. 
			// So it is considered safe to skip the warning.
			//std::cout << "WARN: bufferView.target is zero" << std::endl;
			continue;
		}

		const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
		GLuint vbo;
		glGenBuffers(1, &vbo);
		glBindBuffer(target, vbo);
		glBufferData(target, bufferView.byteLength,
			&buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);

		vbos[i] = vbo;
	}

	// Each mesh can contain several primitives (or parts), each we need to 
	// bind to an OpenGL vertex array object
	for (size_t i = 0; i < mesh.primitives.size(); ++i) {

		tinygltf::Primitive primitive = mesh.primitives[i];
		tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];
		tinygltf::Material material = model.materials[primitive.material];
		GLuint texture = 0;
		int textureIndex = material.pbrMetallicRoughness.baseColorTexture.index;
		if (textureIndex != -1) {
			tinygltf::Sampler sampler = model.samplers[model.textures[textureIndex].sampler];
			GLenum params[4] = { sampler.magFilter,sampler.minFilter,sampler.wrapS,sampler.wrapT };
			texture = loadTexture(params, texture_file_path);
		}

		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos[0]);

		for (auto& attrib : primitive.attributes) {
			tinygltf::Accessor accessor = model.accessors[attrib.second];
			int byteStride =
				accessor.ByteStride(model.bufferViews[accessor.bufferView]);
			glBindBuffer(GL_ARRAY_BUFFER, vbos[accessor.bufferView]);

			int size = 1;
			if (accessor.type != TINYGLTF_TYPE_SCALAR) {
				size = accessor.type;
			}

			int vaa = -1;
			if (attrib.first.compare("POSITION") == 0) {
				vaa = 0;
			}
			if (attrib.first.compare("NORMAL") == 0) vaa = 1;
			if (attrib.first.compare("TEXCOORD_0") == 0) vaa = 2;
			if (attrib.first.compare("COLOR_0") == 0) vaa = 3;


			if (vaa > -1) {
				glEnableVertexAttribArray(vaa);
				glVertexAttribPointer(vaa, size, accessor.componentType,
					accessor.normalized ? GL_TRUE : GL_FALSE,
					byteStride, BUFFER_OFFSET(accessor.byteOffset));
			}
			else {
				std::cout << "vaa missing: " << attrib.first << std::endl;
			}
		}

		// Record VAO for later use
		PrimitiveObject primitiveObject;
		primitiveObject.vao = vao;
		primitiveObject.texture = texture;
		primitiveObject.vbos = vbos;
		primitiveObjects.push_back(primitiveObject);

		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

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

void getSceneTransform(tinygltf::Model& model, glm::mat4& transform, tinygltf::Node& node) {

	if (node.matrix.size() != 0) {
		glm::mat4 t(node.matrix[0], node.matrix[1], node.matrix[2], node.matrix[3],
			node.matrix[4], node.matrix[5], node.matrix[6], node.matrix[7],
			node.matrix[8], node.matrix[9], node.matrix[10], node.matrix[11],
			node.matrix[12], node.matrix[13], node.matrix[14], node.matrix[15]);
		transform = transform * t;
	}
	for (int child : node.children) {
		getSceneTransform(model, transform, model.nodes[child]);
	}
}

bool loadModel(tinygltf::Model& model, const char* filename) {
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;

	bool res = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
	if (!warn.empty()) {
		std::cout << "WARN: " << warn << std::endl;
	}

	if (!err.empty()) {
		std::cout << "ERR: " << err << std::endl;
	}

	if (!res)
		std::cout << "Failed to load glTF: " << filename << std::endl;
	else
		std::cout << "Loaded glTF: " << filename << std::endl;

	return res;
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
	model.mvpMatrixID = glGetUniformLocation(model.shader, "MVP");
	model.textureSampler = glGetUniformLocation(model.shader, "textureSampler");
}

void drawMesh(const std::vector<PrimitiveObject>& primitiveObjects,
	tinygltf::Model& model,GLuint textureSampler,tinygltf::Mesh& mesh) {
	for (size_t i = 0; i < mesh.primitives.size(); ++i)
	{
		GLuint vao = primitiveObjects[i].vao;
		std::map<int, GLuint> vbos = primitiveObjects[i].vbos;
		GLuint texture = primitiveObjects[i].texture;
		glBindVertexArray(vao);
		if (texture != 0) {
			glActiveTexture(GL_TEXTURE0);
			glUniform1f(textureSampler, 0);
			glBindTexture(GL_TEXTURE_2D, texture);
		}

		tinygltf::Primitive primitive = mesh.primitives[i];
		tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];
		tinygltf::BufferView bv = model.bufferViews[indexAccessor.bufferView];
		glBindBuffer(bv.target, vbos[indexAccessor.bufferView]);
		glDrawElements(primitive.mode, indexAccessor.count,
			indexAccessor.componentType,
			BUFFER_OFFSET(indexAccessor.byteOffset));
	}
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
	glUniformMatrix4fv(model.mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);
	drawModel(model);
}