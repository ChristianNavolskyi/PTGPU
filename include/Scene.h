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
	cl_float3 emittance;
	cl_float3 surfaceCharacteristic;
} Sphere;

typedef struct LightSphere
{
	cl_int sphereId;
	cl_int radiance;
} LightSphere;

typedef struct Triangle
{
	cl_float3 p1, p2, p3;
	cl_float3 color;
	cl_float3 emittance;
	cl_float3 surfaceCharacteristic;
};

typedef struct LightTriangle
{
	cl_int triangleId;
	cl_int radiance;
};

typedef struct SceneInfo
{
	cl_int sphereCount;
	cl_int lightSphereCount;
	cl_int triangleCount;
	cl_int lightTriangleCount;
	cl_int totalRadiance;
	cl_float3 backgroundColor;
} SceneInfo;

enum SceneMovementDirections
{
	UP, DOWN, LEFT, RIGHT, FORWARD, BACKWARD, YAW_LEFT, YAW_RIGHT, PITCH_DOWN, PITCH_UP
};

class Scene
{
private:
	std::vector<Sphere> spheres;
	std::vector<LightSphere> lightSpheres;

	std::vector<Triangle> triangles;
	std::vector<LightTriangle> lightTriangles;

	SceneInfo sceneInfo;

	RenderInfoListener *changeListener = nullptr;

	InteractiveCamera *camera = nullptr;

	void notifyListenerCameraChanged();

	void notifyListenerResolutionChanged(int width, int height);

public:
	Scene();

	~Scene();

	void setCamera(InteractiveCamera *camera);

	void addSphere(float radius, glm::vec3 position, glm::vec3 color, glm::vec3 emittance = glm::vec3(0.f), float diffuse = 1.f, float specular = 0.f, float transmissive = 0.f);

	void addTriangle(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 color, glm::vec3 emittance = glm::vec3(0.f), float diffuse = 1.f, float specular = 0.f, float transmissive = 0.f);

	void addPlane(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4, glm::vec3 color, glm::vec3 emittance = glm::vec3(0.f), float diffuse = 1.f, float specular = 0.f, float transmissive = 0.f);

	void setBackgroundColor(float r, float g, float b);

	void linkUpdateListener(RenderInfoListener *listener);

	Sphere *getSphereData();

	size_t getSphereSize();

	LightSphere *getLightSphereData();

	size_t getLightSphereSize();

	Triangle *getTriangleData();

	size_t getTriangleSize();

	LightTriangle *getLightTriangleData();

	size_t getLightTriangleSize();

	SceneInfo *getSceneInfo();

	void initialMousePosition(float xPos, float yPos);

	void updateMousePosition(float xPos, float yPos);

	void changeResolution(int width, int height);

	void move(SceneMovementDirections direction);

	glm::ivec2 getResolution();

	Camera getRenderCamera();
};


