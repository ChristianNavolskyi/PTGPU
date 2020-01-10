//
// Created by Christian Navolskyi on 04.01.20.
//

#pragma once

#include <OpenCL/cl.h>
#include "Scene.h"
#include "InteractiveCamera.h"
#include "RenderInfoListener.h"

/**
 * This class contains all the OpenCL context information, starts the path tracing execution and returns the result
 */
class CLTracer : public RenderInfoListener
{
private:
	cl_platform_id platformId;
	cl_device_id deviceId;
	cl_context context;
	cl_command_queue commandQueue;

	cl_kernel renderKernel;
	cl_kernel clearKernel;
	cl_kernel initializeRenderPlaneKernel;
	cl_program program;

	cl_mem image;
	cl_mem imagePlane;
	cl_mem spheres;
	cl_int sphereCount;
	cl_mem camera;

	GLuint colorTargetId;
	GLuint vertexTargetId;

	cl_int iteration = 0;
	size_t localWorkSize[2];
	size_t globalWorkSize[2];
	Scene scene;

	int width, height;

	void loadPlatformAndDevice();

	void loadContext();

	void loadCommandQueue();

	void loadProgram(const char *programPath);

	void loadKernel(cl_kernel *kernel, const char *kernelName);

	void clearImage();

	void initializeRenderPlane();

	void setGlobalWorkSize();

	void initScene();

	void initializeRenderTargets();

public:
	explicit CLTracer(Scene scene, const size_t localWorkSize[2]);

	~CLTracer();

	bool init(const char *programPath);

	void linkOpenGLResources(GLuint vertexTargetId, GLuint colorTargetId);

	void changeScene(Scene scene);

	void trace();

	void resetRendering();

	void notify() override;

	void notifySizeChanged(int newWidth, int newHeight) override;

	void allocateFixedCLBuffers();

	void writeSceneBuffer();

	void writeCameraBuffer();

	void setSizeArgs();

	void initAppleCLGL();
};
