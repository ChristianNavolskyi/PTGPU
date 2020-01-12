//
// Created by Christian Navolskyi on 08.01.20.
//

#pragma once

#include <glm/glm.hpp>
#include <OpenCL/opencl.h>

struct Camera
{
	cl_float3 position;
	cl_float3 view;
	cl_float3 up;
	cl_float2 fov;
	cl_int2 resolution;
};

class InteractiveCamera
{
private:
	glm::ivec2 resolution;
	glm::vec2 fov;
	glm::vec3 centerPosition;
	glm::vec3 viewDirection;
	glm::vec3 strafeAxis;
	float yaw;
	float pitch;
	float radius;

	float lastXPos = 0.f, lastYPos = 0.f;

	 float padding = 0.001f;

	glm::vec3 setupViewDirection();

public:
	InteractiveCamera(int width, int height);

	~InteractiveCamera();

	Camera createRenderCamera();

	void setResolution(int width, int height);

	glm::ivec2 getResolution();

	void setInitialMousePosition(float xPos, float yPos);

	void handleMouseMovement(float xPos, float yPos);

	void goForward(float delta);

	void strafe(float delta);

	void changeAltitude(float delta);

	void updateStrafeAxis();

	void changeYaw(float delta);

	void changePitch(float delta);
};


