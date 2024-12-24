#include "Utils.h"
#include <string> 
#include <stb_image.h>
#include <stb_image_write.h>
#include <iostream> 
#include <fstream>
#include <sstream> 
#include <vector>


;

void initializeFrameBuffer(Light & l) {
	l.programID = LoadShadersFromString(l.depth_vertex_shader, l.depth_fragment_shader);
	glGenFramebuffers(1, &l.frameBufferID);
	glGenTextures(1, &l.shadowMapID);
	glBindTexture(GL_TEXTURE_2D, l.shadowMapID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		1920, 1080, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindFramebuffer(GL_FRAMEBUFFER, l.frameBufferID);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, l.shadowMapID, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "Frame buffer object not complete";
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	l.depthMatrixID = glGetUniformLocation(l.programID, "depthMVP");
}

GLuint loadTexture(GLenum* params, const char* img_path) {
	int w, h, channels;
	unsigned char* img = stbi_load(img_path, &w, &h, &channels, 3);

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, params[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, params[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, params[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, params[3]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
	glGenerateMipmap(GL_TEXTURE_2D);

	free(img);
	return texture;
}

void CalculateSurfaceNormals(GLfloat* out, size_t length,GLfloat* vertices) {
	for (int i = 0; i < length; i += 12) {
		glm::vec3 side1(vertices[i] - vertices[i + 3], vertices[i + 1] - vertices[i + 4], vertices[i + 2] - vertices[i + 5]);
		glm::vec3 side2(vertices[i] - vertices[i + 6], vertices[i + 1] - vertices[i + 7], vertices[i + 2] - vertices[i + 8]);

		glm::vec3 normalized = glm::normalize(glm::cross(side1, side2));
		out[i] = normalized[0];
		out[i + 1] = normalized[1];
		out[i + 2] = normalized[2];

		out[i + 3] = normalized[0];
		out[i + 4] = normalized[1];
		out[i + 5] = normalized[2];

		out[i + 6] = normalized[0];
		out[i + 7] = normalized[1];
		out[i + 8] = normalized[2];

		out[i + 9] = normalized[0];
		out[i + 10] = normalized[1];
		out[i + 11] = normalized[2];
	}
}

GLuint LoadShadersFromFile(const char* vertex_file_path, const char* fragment_file_path)
{
	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open())
	{
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}
	else
	{
		printf("Vertex shader not found %s.\n", vertex_file_path);
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if (FragmentShaderStream.is_open())
	{
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}
	else
	{
		printf("Fragment shader not found %s.\n", fragment_file_path);
		return 0;
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling vertex shader : %s\n", vertex_file_path);
	char const* VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	if (!Result) {
		printf("Error compiling vertex shader : %s\n", vertex_file_path);
		glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (InfoLogLength > 0) {
			std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
			glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
			printf("%s\n", &VertexShaderErrorMessage[0]);
		}
		return 0;
	}

	// Compile Fragment Shader
	printf("Compiling fragment shader : %s\n", fragment_file_path);
	char const* FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	if (!Result) {
		printf("Error compiling fragment shader : %s\n", fragment_file_path);
		glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (InfoLogLength > 0)
		{
			std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
			glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
			printf("%s\n", &FragmentShaderErrorMessage[0]);
		}
		return 0;
	}

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	if (!Result) {
		printf("Error linking program\n");
		glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (InfoLogLength > 0)
		{
			std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
			glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
			printf("%s\n", &ProgramErrorMessage[0]);
		}
		return 0;
	}

	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

GLuint LoadShadersFromString(std::string VertexShaderCode, std::string FragmentShaderCode)
{
	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling vertex shader\n");
	char const* VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	if (!Result) {
		printf("Error compiling vertex shader\n");
		glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (InfoLogLength > 0) {
			std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
			glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
			printf("%s\n", &VertexShaderErrorMessage[0]);
		}
		return 0;
	}

	// Compile Fragment Shader
	printf("Compiling fragment shader\n");
	char const* FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	if (!Result) {
		printf("Error compiling fragment shader\n");
		glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (InfoLogLength > 0)
		{
			std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
			glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
			printf("%s\n", &FragmentShaderErrorMessage[0]);
		}
		return 0;
	}

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	if (!Result) {
		printf("Error linking program\n");
		glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (InfoLogLength > 0)
		{
			std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
			glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
			printf("%s\n", &ProgramErrorMessage[0]);
		}
		return 0;
	}

	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

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
		tinygltf::Accessor indexAccessor = model.accessors[mesh.primitives[0].indices];
		// Record VAO for later use
		PrimitiveObject primitiveObject;
		primitiveObject.vao = vao;
		primitiveObject.texture = texture;
		primitiveObject.vbos = vbos;
		primitiveObject.num_indices = indexAccessor.count;
		primitiveObjects.push_back(primitiveObject);

		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void getSceneTransform(tinygltf::Model& model, glm::mat4& transform, tinygltf::Node& node) {
	glm::mat4 t(1.0f);
	if (node.matrix.size() != 0) {
		glm::mat4 t = glm::make_mat4(node.matrix.data());
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

void drawMesh(const std::vector<PrimitiveObject>& primitiveObjects,
	tinygltf::Model& model, GLuint textureSampler, tinygltf::Mesh& mesh) {
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
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos.at(indexAccessor.bufferView));
		glDrawElements(primitive.mode, indexAccessor.count,
			indexAccessor.componentType,
			BUFFER_OFFSET(indexAccessor.byteOffset));
		glBindVertexArray(0);
	}
}

void saveDepthTexture(GLuint fbo, std::string filename) {
	int width = 1920;
	int height = 1080;
	int channels = 3;

	std::vector<float> depth(width * height);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glReadBuffer(GL_DEPTH_COMPONENT);
	glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, depth.data());
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	std::vector<unsigned char> img(width * height * 3);
	for (int i = 0; i < width * height; ++i) img[3 * i] = img[3 * i + 1] = img[3 * i + 2] = depth[i] * 255;

	stbi_write_png(filename.c_str(), width, height, channels, img.data(), width * channels);
}

void renderToShadowMap(Light& l, GLuint& vao, glm::mat4 depthMatrix, int indices) {
	glBindVertexArray(vao);
	glm::mat4 lightProjection = glm::perspective(45.0f, 16.0f / 9.0f, 100.0f, 5000.0f);
	glm::mat4 lightVP = lightProjection * glm::lookAt(l.position, l.position + l.front, glm::vec3(0, 1, 0));
	glm::mat4 lightMVP = lightVP * depthMatrix;
	glUniformMatrix4fv(l.depthMatrixID, 1, GL_FALSE, &lightMVP[0][0]);
	glDrawElements(
		GL_TRIANGLES,
		indices,
		GL_UNSIGNED_INT,
		(void*)0
	);
	glBindVertexArray(0);
}

glm::mat4 getLightMVP(Light& light,glm::mat4 m) {
	glm::mat4 lightPerspective = glm::perspective(glm::radians(45.0f), (float)16 / 9, 100.0f, 5000.0f);
	glm::mat4 lightCamera = glm::lookAt(light.position, light.position + light.front, glm::vec3(0, 1, 0));
	return lightPerspective * lightCamera * m;
}
