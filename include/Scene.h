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
	cl_float3 color;
	cl_float4 emittance;
	cl_float3 surfaceCharacteristic;
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

	void addSphere(float xPos, float yPos, float zPos, float radius, float rColor, float gColor, float bColor, float rEmittance = 0.f, float gEmittance = 0.f, float bEmittance = 0.f, float diffuse = 1.f, float specular = 0.f,
				   float transmissive = 0.f);

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


