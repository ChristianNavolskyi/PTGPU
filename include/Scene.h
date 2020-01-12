//
// Created by Christian Navolskyi on 07.01.20.
//

#pragma once


#include <vector>
#include <OpenCL/opencl.h>
#include "InteractiveCamera.h"
#include "RenderInfoListener.h"

typedef struct Sphere
{
	cl_float radius;
	cl_float dummy0;
	cl_float dummy1;
	cl_float dummy2;
	cl_float3 position;
	cl_float4 color;
	cl_float4 emittance;
} Sphere;

enum SceneMovementDirections
{
	UP, DOWN, LEFT, RIGHT, FORWARD, BACKWARD, YAW_LEFT, YAW_RIGHT, PITCH_DOWN, PITCH_UP
};

class Scene
{
private:
	std::vector<Sphere> spheres;
	RenderInfoListener *changeListener = nullptr;

	void notifyListenerCameraChanged();

	void notifyListenerResolutionChanged(int width, int height);

public:
	Scene(int width, int height);

	~Scene();

	void addSphere(float xPos, float yPos, float zPos, float radius, float rColor, float gColor, float bColor, float rEmittance, float gEmittance, float bEmittance);

	void linkUpdateListener(RenderInfoListener *listener);

	size_t getSphereSize();

	const Sphere *getSphereData();

	cl_int getSphereCount();

	void initialMousePosition(float xPos, float yPos);

	void updateMousePosition(float xPos, float yPos);

	void changeResolution(int width, int height);

	void move(SceneMovementDirections direction);

	glm::ivec2 getResolution();

	Camera getRenderCamera();

	InteractiveCamera camera;
};


