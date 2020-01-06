//
// Created by Melina Christian Navolskyi on 04.01.20.
//

#pragma once

#include <OpenCL/cl.h>

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

	int width, height;
	size_t localWorkSize[2];

	void loadPlatformAndDevice();
	void loadContext();
	void loadCommandQueue();
	void loadProgram(const char* programPath);
	void loadKernel(const char* kernelName);
public:
	CLTracer(const size_t localWorkSize[2]);
	~CLTracer();

	bool init(const char* programPath, const char* kernelName);
	void loadImage(float* imageData, int imageWidth, int imageHeight);
	void trace(float* resultImage);
};
