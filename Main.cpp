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


static GLFWwindow* window;
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

static glm::vec3 eye_center;
static glm::vec3 lookat;
static glm::vec3 up(0, 1, 0);

static float viewAzimuth = 0.f;
static float viewPolar = 0.f;
static float viewDistance = 300.0f;

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
	glfwSetKeyCallback(window, key_callback);

	glEnable(GL_DEPTH_TEST);

	eye_center.y = viewDistance * cos(viewPolar);
	eye_center.x = viewDistance * cos(viewAzimuth);
	eye_center.z = viewDistance * sin(viewAzimuth);

	Model m;
	m.gtlf_file_path = "../assets/gold_mine/scene.gltf";
	m.texture_file_path = "../assets/gold_mine/textures/TerrainNodeMaterial_baseColor.jpeg";
    initializeModel(m);

	SkyBox sky;
	sky.scale = glm::vec3(2000, 2000, 2000);
	sky.position = eye_center;
	sky.texture_file_path = "../assets/textures/sky.png";
	initialize(sky);

	City city;
	city.position = glm::vec3(-30, 2, 0);
	city.scale = glm::vec3(5, 10, 5);
	initializeCity(city, 10);

    // Camera setup
   

    glm::mat4 viewMatrix, projectionMatrix;
    glm::float32 FoV = 45;
    glm::float32 zNear = 100.0f;
    glm::float32 zFar = 5000.0f;
    projectionMatrix = glm::perspective(glm::radians(FoV), 16.0f / 9.0f, zNear, zFar);
	
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		viewMatrix = glm::lookAt(eye_center, lookat, up);
		glm::mat4 vp = projectionMatrix * viewMatrix;

        /* Render here */
		renderModel(m,vp);
		render(sky, vp);
		renderCity(city,vp);

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
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		viewAzimuth = 0.f;
		viewPolar = 0.f;
		eye_center.y = viewDistance * cos(viewPolar);
		eye_center.x = viewDistance * cos(viewAzimuth);
		eye_center.z = viewDistance * sin(viewAzimuth);
		std::cout << "Reset." << std::endl;
	}

	if (key == GLFW_KEY_UP && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		viewPolar -= 0.1f;
		eye_center.y = viewDistance * cos(viewPolar);
		eye_center.x = viewDistance * cos(viewAzimuth);
		eye_center.z = viewDistance * sin(viewAzimuth);
	}

	if (key == GLFW_KEY_DOWN && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		viewPolar += 0.1f;
		eye_center.y = viewDistance * cos(viewPolar);
		eye_center.x = viewDistance * cos(viewAzimuth);
		eye_center.z = viewDistance * sin(viewAzimuth);
	}

	if (key == GLFW_KEY_W) {
		eye_center.z += 10.0f;
		lookat.z += 10.0f;

	}
	if (key == GLFW_KEY_S) {
		eye_center.z -= 10.0f;
		lookat.z -= 10.0f;
	}
	if (key == GLFW_KEY_A) {
		eye_center.x -= 10.0f;
		lookat.x -= 10.0f;
	}
	if (key == GLFW_KEY_D) {
		eye_center.x += 10.0f;
		lookat.x += 10.0f;
	}
	if (key == GLFW_KEY_Q) {
		eye_center.y += 10.0f;
		lookat.y += 10.0f;
	}
	if (key == GLFW_KEY_E) {
		eye_center.y -= 10.0f;
		lookat.y -= 10.0f;
	}

	if (key == GLFW_KEY_LEFT && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		viewAzimuth -= 0.1f;
		eye_center.x = viewDistance * cos(viewAzimuth);
		eye_center.z = viewDistance * sin(viewAzimuth);
	}

	if (key == GLFW_KEY_RIGHT && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		viewAzimuth += 0.1f;
		eye_center.x = viewDistance * cos(viewAzimuth);
		eye_center.z = viewDistance * sin(viewAzimuth);
	}

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	//std::cout << eye_center.x << ',' << eye_center.y << ',' << eye_center.z << '\n';
}