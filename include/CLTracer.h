//
// Created by Melina Christian Navolskyi on 04.01.20.
//

#pragma once

#include <OpenCL/cl.h>
#include "Scene.h"

/**
 * This class contains all the OpenCL context information, starts the path tracing execution and returns the result
 */
class CLTracer
{
private:
	cl_platform_id platformId;
	cl_device_id deviceId;
	cl_context context;
	cl_command_queue commandQueue;

	cl_kernel kernel;
	cl_program program;

	cl_mem image;
	cl_mem spheres;

	cl_int width;
	cl_int height;
	cl_int iteration = 0;
	size_t localWorkSize[2];
	Scene scene;

	cl_float3 cameraPosition; // TODO incorporate
	cl_float3 viewDirection;

	void loadPlatformAndDevice();

	void loadContext();

	void loadCommandQueue();

	void loadProgram(const char *programPath);

	void loadKernel(const char *kernelName);

	void initScene();

public:
	explicit CLTracer(Scene scene, const size_t localWorkSize[2]);

	~CLTracer();

	bool init(const char *programPath, const char *kernelName);

	void changeScene(Scene scene);

	void setImageSize(int imageWidth, int imageHeight);

	void trace(float *imageData);

	void addGLTexture(GLenum textureTarget, GLuint textureId);
};
