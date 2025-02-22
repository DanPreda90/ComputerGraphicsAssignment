#pragma once
#include "Plane.h"
#include <string>
#include <iostream>

const glm::vec3 wave500(0.0f, 255.0f, 146.0f);
const glm::vec3 wave600(255.0f, 190.0f, 0.0f);
const glm::vec3 wave700(0.0f, 0.0f, 205.0f);;
static glm::vec3 lightIntensity = 8000.0f * (8.0f * wave500 + 15.6f * wave600 + 18.4f * wave700);
static glm::mat4 m(1.0f);

glm::mat4 getNodeTransform(const tinygltf::Node& node) {
	glm::mat4 transform(1.0f);

	if (node.matrix.size() == 16) {
		transform = glm::make_mat4(node.matrix.data());
	}
	else {
		if (node.translation.size() == 3) {
			transform = glm::translate(transform, glm::vec3(node.translation[0], node.translation[1], node.translation[2]));
		}
		if (node.rotation.size() == 4) {
			glm::quat q(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
			transform *= glm::mat4_cast(q);
		}
		if (node.scale.size() == 3) {
			transform = glm::scale(transform, glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
		}
	}
	return transform;
}

void computeLocalNodeTransform(const tinygltf::Model& model,
	int nodeIndex,
	std::vector<glm::mat4>& localTransforms)
{
	tinygltf::Node currentNode = model.nodes[nodeIndex];
	localTransforms[nodeIndex] = getNodeTransform(currentNode);
	for (int child : currentNode.children) {
		computeLocalNodeTransform(model, child, localTransforms);
	}
}

void computeGlobalNodeTransform(const tinygltf::Model& model,
	const std::vector<glm::mat4>& localTransforms,
	int nodeIndex, const glm::mat4 parentTransform,
	std::vector<glm::mat4>& globalTransforms)
{
	glm::mat4 globalTransform = parentTransform * localTransforms[nodeIndex];
	std::vector<int> children = model.nodes[nodeIndex].children;
	if (children.size() != 0) {
		for (int child : children) {
			computeGlobalNodeTransform(model, localTransforms, child, globalTransform, globalTransforms);
		}
	}
	else {
		globalTransforms[model.nodes[nodeIndex].mesh] = globalTransform;
	}
}

int findKeyframeIndex(const std::vector<float>& times, float animationTime)
{
	int left = 0;
	int right = times.size() - 1;

	while (left <= right) {
		int mid = (left + right) / 2;

		if (mid + 1 < times.size() && times[mid] <= animationTime && animationTime < times[mid + 1]) {
			return mid;
		}
		else if (times[mid] > animationTime) {
			right = mid - 1;
		}
		else { // animationTime >= times[mid + 1]
			left = mid + 1;
		}
	}

	// Target not found
	return times.size() - 2;
}

std::vector<AnimationObject> prepareAnimation(Plane & plane)
{
	std::vector<AnimationObject> animationObjects;
	for (const auto& anim : plane.model->animations) {
		AnimationObject animationObject;

		for (const auto& sampler : anim.samplers) {
			SamplerObject samplerObject;

			const tinygltf::Accessor& inputAccessor = plane.model->accessors[sampler.input];
			const tinygltf::BufferView& inputBufferView = plane.model->bufferViews[inputAccessor.bufferView];
			const tinygltf::Buffer& inputBuffer = plane.model->buffers[inputBufferView.buffer];

			assert(inputAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
			assert(inputAccessor.type == TINYGLTF_TYPE_SCALAR);

			// Input (time) values
			samplerObject.input.resize(inputAccessor.count);

			const unsigned char* inputPtr = &inputBuffer.data[inputBufferView.byteOffset + inputAccessor.byteOffset];
			const float* inputBuf = reinterpret_cast<const float*>(inputPtr);

			// Read input (time) values
			int stride = inputAccessor.ByteStride(inputBufferView);
			for (size_t i = 0; i < inputAccessor.count; ++i) {
				samplerObject.input[i] = *reinterpret_cast<const float*>(inputPtr + i * stride);
			}

			const tinygltf::Accessor& outputAccessor = plane.model->accessors[sampler.output];
			const tinygltf::BufferView& outputBufferView = plane.model->bufferViews[outputAccessor.bufferView];
			const tinygltf::Buffer& outputBuffer = plane.model->buffers[outputBufferView.buffer];

			assert(outputAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

			const unsigned char* outputPtr = &outputBuffer.data[outputBufferView.byteOffset + outputAccessor.byteOffset];
			const float* outputBuf = reinterpret_cast<const float*>(outputPtr);

			int outputStride = outputAccessor.ByteStride(outputBufferView);

			// Output values
			samplerObject.output.resize(outputAccessor.count);

			for (size_t i = 0; i < outputAccessor.count; ++i) {

				if (outputAccessor.type == TINYGLTF_TYPE_VEC3) {
					memcpy(&samplerObject.output[i], outputPtr + i * 3 * sizeof(float), 3 * sizeof(float));
				}
				else if (outputAccessor.type == TINYGLTF_TYPE_VEC4) {
					memcpy(&samplerObject.output[i], outputPtr + i * 4 * sizeof(float), 4 * sizeof(float));
				}
				else {
					std::cout << "Unsupport accessor type ..." << std::endl;
				}

			}

			animationObject.samplers.push_back(samplerObject);
		}

		animationObjects.push_back(animationObject);
	}
	return animationObjects;
}

std::vector<int> getMeshIndices(const tinygltf::Model& model, int target) {
	tinygltf::Node node = model.nodes[target];
	std::vector<int> res;

	for (int child : node.children) {
		tinygltf::Node ch = model.nodes[child];
		if (ch.mesh != -1) {
			res.push_back(ch.mesh);
		}
	}
	return res;
}

void updateAnimation(
	const tinygltf::Model& model,
	const tinygltf::Animation& anim,
	const AnimationObject& animationObject,
	float time,
	std::vector<glm::mat4>& nodeTransforms)
{
	// There are many channels so we have to accumulate the transforms 
	for (const auto& channel : anim.channels) {

		int targetNodeIndex = channel.target_node;
		const auto& sampler = anim.samplers[channel.sampler];

		// Access output (value) data for the channel
		const tinygltf::Accessor& outputAccessor = model.accessors[sampler.output];
		const tinygltf::BufferView& outputBufferView = model.bufferViews[outputAccessor.bufferView];
		const tinygltf::Buffer& outputBuffer = model.buffers[outputBufferView.buffer];

		// Calculate current animation time (wrap if necessary)
		const std::vector<float>& times = animationObject.samplers[channel.sampler].input;
		float animationTime = fmod(time, times.back());

		// ----------------------------------------------------------
		// TODO: Find a keyframe for getting animation data 
		// ----------------------------------------------------------
		int keyframeIndex = findKeyframeIndex(times, animationTime);

		const unsigned char* outputPtr = &outputBuffer.data[outputBufferView.byteOffset + outputAccessor.byteOffset];
		const float* outputBuf = reinterpret_cast<const float*>(outputPtr);

		// -----------------------------------------------------------
		// TODO: Add interpolation for smooth animation
		// -----------------------------------------------------------
		glm::mat4 t(1.0f);
		if (channel.target_path == "translation") {
			glm::vec3 translation0, translation1;
			glm::vec3 translation;
			memcpy(&translation0, outputPtr + keyframeIndex * 3 * sizeof(float), 3 * sizeof(float));
			if (keyframeIndex + 1 < times.size()) {
				memcpy(&translation1, outputPtr + (keyframeIndex + 1) * 3 * sizeof(float), 3 * sizeof(float));
				float interpolationValue = (float)(animationTime - times[keyframeIndex]) / (float)(times[keyframeIndex + 1] - times[keyframeIndex]);
				translation = translation0 + (interpolationValue * (translation1 - translation0));
			}
			else {
				translation = translation0;
			}

			t = glm::translate(t, translation);
		}
		else if (channel.target_path == "rotation") {
			glm::quat rotation0, rotation1;
			glm::quat rotation;
			memcpy(&rotation0, outputPtr + keyframeIndex * 4 * sizeof(float), 4 * sizeof(float));
			if (keyframeIndex + 1 < times.size()) {
				memcpy(&rotation1, outputPtr + (keyframeIndex + 1) * 4 * sizeof(float), 4 * sizeof(float));
				float interpolationValue = (float)(animationTime - times[keyframeIndex]) / (float)(times[keyframeIndex + 1] - times[keyframeIndex]);
				rotation = glm::slerp<float>(rotation0, rotation1, interpolationValue);
			}
			else {
				rotation = rotation0;
			}
			t *= glm::mat4_cast(rotation);
		}
		else if (channel.target_path == "scale") {
			glm::vec3 scale0, scale1;
			glm::vec3 scale;
			memcpy(&scale0, outputPtr + keyframeIndex * 3 * sizeof(float), 3 * sizeof(float));
			if (keyframeIndex + 1 < times.size()) {
				memcpy(&scale1, outputPtr + (keyframeIndex + 1) * 3 * sizeof(float), 3 * sizeof(float));
				float interpolationValue = (float)(animationTime - times[keyframeIndex]) / (float)(times[keyframeIndex + 1] - times[keyframeIndex]);
				scale = scale0 + (interpolationValue * (scale1 - scale0));
			}
			else {
				scale = scale0;
			}
			t = glm::scale(t, scale);
		}
		nodeTransforms[channel.target_node] = t;
	}
}

void update(Plane & p,float time)
{
	//return;
	
	if (p.model->animations.size() > 0) {
		const tinygltf::Animation& animation = p.model->animations[0];
		const AnimationObject& animationObject = p.animationObjects[0];
		
		std::vector<glm::mat4> nodeTransforms(p.model->nodes.size());
		std::vector<glm::mat4> animationTransforms(p.model->nodes.size());
		computeLocalNodeTransform(*p.model, 0, nodeTransforms);
		updateAnimation(*p.model, animation, animationObject, time, animationTransforms);
		for (int i = 0; i < nodeTransforms.size(); ++i) {
			nodeTransforms[i] = nodeTransforms[i] * animationTransforms[i];
		}
		
		for (const tinygltf::AnimationChannel& channel : animation.channels) {
			computeGlobalNodeTransform(*p.model, nodeTransforms, channel.target_node, glm::mat4(1.0f), p.meshMatrices);
		}
	}
	
}

void bindPlaneMesh(std::vector<PrimitiveObject>& primitiveObjects,
	tinygltf::Model& model, const char* texture_file_path, tinygltf::Mesh& mesh) {
	std::map<int, GLuint> vbos;
	int mesh_index = -1;
	for (int j = 0; j < model.meshes.size(); ++j) {
		if (model.meshes[j] == mesh) {
			mesh_index = j;
			break;
		}
	}
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
		tinygltf::Material material = model.materials[primitive.material];
		GLuint texture = 0;
		int textureIndex = material.pbrMetallicRoughness.baseColorTexture.index;
		if (textureIndex != -1) {
			tinygltf::Texture tex = model.textures[textureIndex];
			tinygltf::Sampler sampler = model.samplers[tex.sampler];
			std::string img_uri = model.images[tex.source].uri;
			std::string path = std::string(texture_file_path) + img_uri;
			GLenum params[4] = { sampler.magFilter,sampler.minFilter,sampler.wrapS,sampler.wrapT };
			texture = loadTexture(params, path.c_str());
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
			if (attrib.first.compare("TANGENT") == 0) vaa = 3;

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
		tinygltf::Accessor indexAccessor = model.accessors[mesh.primitives[0].indices];
		// Record VAO for later use
		PrimitiveObject primitiveObject;
		primitiveObject.vao = vao;
		primitiveObject.texture = texture;
		primitiveObject.vbos = vbos;
		primitiveObject.num_indices = indexAccessor.count;
		primitiveObject.mesh_index = mesh_index;
		primitiveObjects.push_back(primitiveObject);

		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void bindModelNodes(Plane& m,
	tinygltf::Node& node) {
	// Bind buffers for the current mesh at the node
	if ((node.mesh >= 0) && (node.mesh < m.model->meshes.size())) {
		std::vector<PrimitiveObject> primitives;
		bindPlaneMesh(primitives, *m.model, m.texture_file_path, m.model->meshes[node.mesh]);
		m.primitiveObjects.push_back(primitives);
	}

	// Recursive into children nodes
	for (size_t i = 0; i < node.children.size(); i++) {
		assert((node.children[i] >= 0) && (node.children[i] < m.model->nodes.size()));
		bindModelNodes(m, m.model->nodes[node.children[i]]);
	}
}

void bindModel(Plane& m) {
	tinygltf::Scene& scene = m.model->scenes[m.model->defaultScene];
	for (int i = 0; i < scene.nodes.size(); ++i) {
		bindModelNodes(m, m.model->nodes[scene.nodes[i]]);
	}
}

void initalizePlane(Plane& plane,tinygltf::Model * model) {
	plane.shader = LoadShadersFromString(plane.vertexShader, plane.fragmentShader);
	plane.model = model;
	if (!plane.shader) { std::cerr << "Failed to load shaders." << std::endl; }
	tinygltf::Scene scene = plane.model->scenes[0];
	bindModel(plane);
	getSceneTransform(*plane.model, plane.model_transform, plane.model->nodes[plane.model->defaultScene]);
	plane.animationObjects = prepareAnimation(plane);
	for (int i = 0; i < plane.model->meshes.size(); ++i) {
		plane.meshMatrices.push_back(glm::mat4(1.0f));
	}

	plane.lightIntensityID = glGetUniformLocation(plane.shader, "lightIntensity");
	plane.mvpMatrixID = glGetUniformLocation(plane.shader, "MVP");
	plane.depthMapSamplerID = glGetUniformLocation(plane.shader, "depthMapSampler");
	plane.textureSampler = glGetUniformLocation(plane.shader, "textureSampler");
	plane.lightPositionID = glGetUniformLocation(plane.shader, "lightPosition");
}

void drawPlaneMesh(const std::vector<PrimitiveObject>& primitiveObjects,
	tinygltf::Model& model, GLuint textureSampler, tinygltf::Mesh& mesh,Plane & plane,glm::mat4 & vp) {
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
		
		glm::mat4 t = glm::rotate(glm::mat4(1.0f), glm::radians(plane.rotation), glm::vec3(0, 1, 0));
		glm::mat4 mvp = vp * m * t * plane.meshMatrices[primitiveObjects[i].mesh_index];
		glUniformMatrix4fv(plane.mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);
		tinygltf::Primitive primitive = mesh.primitives[i];
		tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];
		tinygltf::BufferView bv = model.bufferViews[indexAccessor.bufferView];
		//glBindBuffer(bv.target, vbos[indexAccessor.bufferView]);
		glDrawElements(primitive.mode, indexAccessor.count,
			indexAccessor.componentType,
			BUFFER_OFFSET(indexAccessor.byteOffset));
		glBindVertexArray(0);
	}
}

void drawModelNodes(Plane& m, tinygltf::Node& node,glm::mat4 & vp) {
	// Draw the mesh at the node, and recursively do so for children nodes
	if ((node.mesh >= 0) && (node.mesh < m.model->meshes.size())) {
		drawPlaneMesh(m.primitiveObjects[node.mesh], *m.model, m.textureSampler, m.model->meshes[node.mesh],m,vp);
		return;
	}
	for (size_t i = 0; i < node.children.size(); i++) {
		drawModelNodes(m, m.model->nodes[node.children[i]],vp);
	}
}
void drawModel(Plane& m,glm::mat4 & vp) {
	// Draw all nodes
	const tinygltf::Scene& scene = m.model->scenes[m.model->defaultScene];
	for (size_t i = 0; i < scene.nodes.size(); ++i) {
		drawModelNodes(m, m.model->nodes[scene.nodes[i]],vp);
	}
}

void renderPlane(Plane& plane, glm::mat4 vp,Light & light) {
	glUseProgram(plane.shader);
	// Scale the box along each axis to make it look like a building
	m = glm::translate(glm::mat4(1.0f), plane.position);
	m = glm::scale(m, plane.scale);
	glm::mat4 lightMVP = getLightMVP(light, m);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, light.depthTextureID);
	glUniform1f(plane.depthMapSamplerID, 0);
	glUniform3fv(plane.lightIntensityID, 1, &lightIntensity[0]);
	glUniform3fv(plane.lightPositionID, 1, &light.position[0]);
	drawModel(plane,vp);
}

void renderPlaneShadow(Plane& plane, Light& light) {
	for (std::vector<PrimitiveObject>& obj : plane.primitiveObjects) {
		m = glm::translate(glm::mat4(1.0f), plane.position);
		m = glm::scale(m, plane.scale);
		glm::mat4 t = glm::rotate(glm::mat4(1.0f), glm::radians(plane.rotation), glm::vec3(0, 1, 0));
		glm::mat4 model = m * t * plane.meshMatrices[obj[0].mesh_index];
		renderToShadowMap(light, obj[0].vao, model, obj[0].num_indices);
	}
}