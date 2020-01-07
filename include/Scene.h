//
// Created by Melina Christian Navolskyi on 07.01.20.
//

#pragma once


#include <vector>
#include <OpenCL/opencl.h>

struct Sphere
{
	cl_float radius;
	cl_float dummy0;
	cl_float dummy1;
	cl_float dummy2;
	cl_float3 position;
	cl_float3 color;
	cl_float3 emittance;
};

class Scene
{
private:
	std::vector<Sphere> spheres;

public:
	Scene();
	~Scene();

	void addSphere(float xPos, float yPos, float zPos, float radius, float rColor, float gColor, float bColor, float rEmittance, float gEmittance, float bEmittance)
	{
		Sphere sphere{};
		sphere.radius = radius;
		sphere.position = {{xPos, yPos, zPos}};
		sphere.color = {{rColor, gColor, bColor}};
		sphere.emittance = {{rEmittance, gEmittance, bEmittance}};

		spheres.push_back(sphere);
	}


	cl_mem createCLSphereBuffer(cl_context context);

	size_t getSphereSize();

	const void *getSphereData();

	cl_int getSphereCount();
};


