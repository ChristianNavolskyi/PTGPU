#include "InteractiveCamera.h"
#include <algorithm>

InteractiveCamera::InteractiveCamera(int width, int height) : centerPosition(0.f, 0.f, 0.f),
															  yaw(0.f),
															  pitch(0.f),
															  radius(1.f),
															  resolution(width, height),
															  fov(40.f, 40.f),
															  viewDirection(),
															  strafeAxis()
{
	setupViewDirection();
}

glm::vec3 InteractiveCamera::setupViewDirection()
{
	float xDirection = sin(yaw) * cos(pitch);
	float yDirection = sin(pitch);
	float zDirection = cos(yaw) * cos(pitch);
	glm::vec3 directionToCamera(xDirection, yDirection, zDirection);

	viewDirection = directionToCamera * -1.0f;

	return directionToCamera;
}

Camera InteractiveCamera::createRenderCamera()
{
	glm::vec3 directionToCamera = setupViewDirection();
	glm::vec3 eyePosition = centerPosition + directionToCamera * radius;

	glm::vec3 view = normalize(glm::vec3(viewDirection.x, viewDirection.y, viewDirection.z));
	updateStrafeAxis();

	Camera camera{};

	camera.position = {{eyePosition.x, eyePosition.y, eyePosition.z}};
	camera.view = {{view.x, view.y, view.z}};
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

glm::ivec2 InteractiveCamera::getResolution()
{
	return resolution;
}

void InteractiveCamera::setInitialMousePosition(float xPos, float yPos)
{
	lastXPos = xPos;
	lastYPos = yPos;
}

void InteractiveCamera::handleMouseMovement(float xPos, float yPos)
{
	float xDelta = lastXPos - xPos;
	float yDelta = lastYPos - yPos;

	lastXPos = xPos;
	lastYPos = yPos;

	changeYaw(-xDelta * 0.01f);
	changePitch(-yDelta * 0.01f);
}

void InteractiveCamera::goForward(float delta)
{
	centerPosition += viewDirection * delta;
}

void InteractiveCamera::strafe(float delta)
{
	centerPosition += strafeAxis * delta;
}

void InteractiveCamera::changeAltitude(float delta)
{
	centerPosition.y += delta;
}

void InteractiveCamera::changeYaw(float delta)
{
	yaw += delta;

	yaw = fmod(yaw, 2 * M_PI);
}

void InteractiveCamera::changePitch(float delta)
{
	pitch += delta;

	pitch = std::min(std::max(pitch, (float) (-M_PI_2 + padding)), (float) (M_PI_2 - padding));
}

void InteractiveCamera::updateStrafeAxis()
{
	strafeAxis = cross(viewDirection, glm::vec3(0.f, 1.f, 0.f));
	strafeAxis = glm::normalize(strafeAxis);
}
