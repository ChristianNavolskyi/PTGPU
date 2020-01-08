//
// Created by Christian Navolskyi on 08.01.20.
//

#include "InteractiveCamera.h"

InteractiveCamera::InteractiveCamera(int width, int height)
{
	centerPosition = glm::vec3(0.f, 0.f, 0.f);
	yaw = 0;
	pitch = 0.3f;
	radius = 4.f;

	resolution.x = width;
	resolution.y = height;

	fov = glm::vec2(40.f, 40.f);
}

InteractiveCamera::~InteractiveCamera()
{

}

Camera InteractiveCamera::createRenderCamera()
{
	Camera camera{};

	float xDirection = sin(yaw) * cos(pitch);
	float yDirection = sin(pitch);
	float zDirection = cos(yaw) * cos(pitch);
	glm::vec3 directionToCamera(xDirection, yDirection, zDirection);
	viewDirection = directionToCamera * -1.0f;
	glm::vec3 eyePosition = centerPosition + directionToCamera * radius;

	camera.position = {{eyePosition.x, eyePosition.y, eyePosition.z}};
	camera.view = {{viewDirection.x, viewDirection.y, viewDirection.z}};
	camera.up = {{0.f, 1.f, 0.f}};
	camera.resolution = {{resolution.x, resolution.y}};
	camera.fov = {{fov.x, fov.y}};

	return camera;
}

void InteractiveCamera::setResolution(int width, int height)
{
	resolution.x = width;
	resolution.y = height;
}

glm::ivec2 InteractiveCamera::getResolution() {
	return resolution;
}
