//
// Created by Melina Christian Navolskyi on 07.01.20.
//

#include "Scene.h"
#include "CLUtil.h"

Scene::Scene() : spheres()
{

}

Scene::~Scene()
{
	spheres.clear();
}

cl_mem Scene::createCLSphereBuffer(cl_context context)
{
	cl_int clError;

	cl_mem sphereMemory = clCreateBuffer(context, CL_MEM_READ_ONLY, getSphereSize(), nullptr, &clError);
	V_RETURN_NULL_CL(clError, "Failed to allocate space for spheres");

	return sphereMemory;
}

size_t Scene::getSphereSize()
{
	return sizeof(Sphere) * spheres.size();
}

const void *Scene::getSphereData()
{
	return &spheres;
}

cl_int Scene::getSphereCount()
{
	return spheres.size();
}
