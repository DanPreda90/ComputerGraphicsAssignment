#pragma once
#include "Utils.h"
struct Camera {
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

	float pitch;
	float yaw;
	float roll;
};

void initializeCamera(Camera & cam,glm::vec3 position, float pitch, float yaw, float roll);
void updateCamera(Camera & cam);