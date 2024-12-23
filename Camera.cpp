#include "Camera.h"

void initializeCamera(Camera & cam,glm::vec3 position,float pitch,float yaw,float roll) {
	cam.position = position;
	cam.pitch = pitch;
	cam.yaw = yaw;
	cam.roll = roll;
	updateCamera(cam);
}

void updateCamera(Camera & cam) {
	glm::vec3 direction;
	direction.x = cos(glm::radians(cam.yaw)) * cos(glm::radians(cam.pitch));
	direction.y = sin(glm::radians(cam.pitch));
	direction.z = sin(glm::radians(cam.yaw)) * cos(glm::radians(cam.pitch));
	cam.front = glm::normalize(direction);
}
