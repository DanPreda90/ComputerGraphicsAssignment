#define GLFW_INCLUDE_NONE
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "Model.h"
#include "SkyBox.h"
#include <glm/gtc/matrix_transform.hpp>
#include "iostream"
#include "City.h"
#include "Plane.h"
#include "Camera.h"
#include "Quad.h"

static GLFWwindow* window;
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
static void click_callback(GLFWwindow* window, int button, int action, int mods);

static tinygltf::Model PlaneModel;

static bool firstMouse = true;
static float lastY = 540;
static float lastX = 960;

const glm::vec3 wave500(0.0f, 255.0f, 146.0f);
const glm::vec3 wave600(255.0f, 190.0f, 0.0f);
const glm::vec3 wave700(0.0f, 0.0f, 255.0f);
static glm::vec3 lightIntensity = 8000.0f * (8.0f * wave500 + 15.6f * wave600 + 18.4f * wave700);

static Camera camera;
static Quad quad;

static void renderShadows(Light & light, City & city, City& city2,Model & m, Plane * planes) {
	glUseProgram(light.programID);
	glBindFramebuffer(GL_FRAMEBUFFER, light.frameBufferID);
	glClear(GL_DEPTH_BUFFER_BIT);
	renderModelShadow(m, light);
	renderCityToShadow(city, light);
	renderCityToShadow(city2, light);
	for (int i = 0; i < 5; ++i) {
		renderPlaneShadow(planes[i], light);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

int main() {
    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Make the window's context current */
    window = glfwCreateWindow(1920, 1080, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, click_callback);
	glfwSetCursorPosCallback(window, mouse_callback);

	glEnable(GL_DEPTH_TEST);
	
	initializeCamera(camera, glm::vec3(13.0711, 1310.4, -610.366), -66.0f, -270.0f, 0.0f);

	Light light;
	light.intensity = lightIntensity;
	light.position = glm::vec3(6.65604, 1187.92, 414.355);
	light.front = glm::vec3(0.000446966, -0.966376, -0.257133);
	initializeFrameBuffer(light);

	Model m;
	m.gtlf_file_path = "../assets/gold_mine/scene.gltf";
	m.texture_file_path = "../assets/gold_mine/";
    initializeModel(m);

	SkyBox sky;
	sky.scale = glm::vec3(2000, 2000, 2000);
	sky.position = &camera.position;
	sky.texture_file_path = "../assets/textures/sky.png";
	initialize(sky);

	quad.position = glm::vec3(0,1,0);
	quad.scale = glm::vec3(5000, 5000, 5000);
	initQuad(quad);

	City city;
	city.position = glm::vec3(-50, 2, 10);
	city.spacing = 5.0f;
	city.scale = glm::vec3(5, 20, 5);
	city.texture_file_path = "../assets/textures/bricks.jpg";
	initializeCity(city, 4);

	City city2;
	city2.position = glm::vec3(40, 5, 20);
	city2.spacing = 5.0f;
	city2.scale = glm::vec3(5, 20, 5);
	city2.texture_file_path = "../assets/textures/bricks.jpg";
	initializeCity(city2, 4);


	loadModel(PlaneModel, "../assets/chernovan_nemesis/scene.gltf");
	Plane planes[5];
	float plane_angles[] = {
		0.0f,
		0.05f,
		0.1f,
		0.05f,
		0.0f
	};

	for (int i = 0; i < ArrayCount(planes); ++i) {
		planes[i].position = glm::vec3(0, 200, 0);
		planes[i].scale = glm::vec3(0.05, 0.05, 0.05);
		planes[i].rotation = plane_angles[i];
		planes[i].texture_file_path = "../assets/chernovan_nemesis/";
		initalizePlane(planes[i], &PlaneModel);
	}
	
    glm::mat4 viewMatrix, projectionMatrix;
    glm::float32 FoV = 45;
    glm::float32 zNear = 100.0f;
    glm::float32 zFar = 5000.0f;
    projectionMatrix = glm::perspective(glm::radians(FoV), 16.0f / 9.0f, zNear, zFar);
	static double lastTime = glfwGetTime();
	float time = 0.0f;			// Animation time 
	float fTime = 0.0f;			// Time for measuring fps
    /* Loop until the user closes the window */
	static float playbackSpeed = 2.0f;
	static float angle = 0.0f;

	unsigned long frames = 0;

    while (!glfwWindowShouldClose(window))
    {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		double currentTime = glfwGetTime();
		float deltaTime = float(currentTime - lastTime);
		lastTime = currentTime;

		
		time += deltaTime * playbackSpeed;
		for (int i = 0; i < ArrayCount(planes); ++i) {
			update(planes[i], time);
		}
		

		viewMatrix = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
		glm::mat4 vp = projectionMatrix * viewMatrix;
	
        /* Render here */
		renderShadows(light, city, city2,m,planes);

		glm::mat4 shadowMVP = projectionMatrix * glm::lookAt(light.position, light.position + light.front, glm::vec3(0, 1, 0));
		renderModel(m,vp,light);
		render(sky, vp);
		renderCity(city,vp,shadowMVP,light);
		renderCity(city2,vp,shadowMVP,light);
		renderQuad(quad, vp);

		int radius = 500;
		
		for (int i = 0; i < ArrayCount(planes); ++i) {
			renderPlane(planes[i], vp,light);
			float x = radius * cos(plane_angles[i]);
			float z = radius * sin(plane_angles[i]);
			planes[i].position.x = x;
			planes[i].position.z = z;
			planes[i].rotation -= 0.05025;
			radius += 50;
			plane_angles[i] += 0.0008;
		}

		frames++;
		fTime += deltaTime;
		if (fTime > 2.0f) {
			float fps = frames / fTime;
			frames = 0;
			fTime = 0;

			char out[200];
			sprintf_s(out,"FPS : %.2f", fps);
			glfwSetWindowTitle(window, out);
		}
		
        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
} 

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	const float cameraSpeed = 10.0f;
	if (key == GLFW_KEY_W) {
		camera.position += cameraSpeed * camera.front;
	}
	if (key == GLFW_KEY_S) {
		camera.position -= cameraSpeed * camera.front;
	}
	if (key == GLFW_KEY_A) {
		camera.position -= glm::normalize(glm::cross(camera.front, camera.up)) * cameraSpeed;
	}
	if (key == GLFW_KEY_D) {
		camera.position += glm::normalize(glm::cross(camera.front, camera.up)) * cameraSpeed;
	}
	if (key == GLFW_KEY_Q){
		camera.position += cameraSpeed * camera.up;
	}
	if (key == GLFW_KEY_E) {
		camera.position -= cameraSpeed * camera.up;
	}
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	//std::cout << eye_center.x << ',' << eye_center.y << ',' << eye_center.z << '\n';
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	float& yaw = camera.yaw;
	float& pitch = camera.pitch;
	float& roll = camera.roll;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;
	updateCamera(camera);
}

void click_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_1) {
		std::cout << camera.position.x << ',' << camera.position.y << ',' << camera.position.z << '\n';
		std::cout << camera.front.x << ',' << camera.front.y << ',' << camera.front.z << '\n';
	}
}