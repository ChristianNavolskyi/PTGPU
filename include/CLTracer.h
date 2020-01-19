//
// Created by Christian Navolskyi on 04.01.20.
//

#pragma once

#include <OpenCL/cl.h>
#include "Scene.h"
#include "InteractiveCamera.h"
#include "RenderInfoListener.h"

enum RenderOption
{
	DEFAULT = 0,
	NORMAL = 1,
	DEPTH = 2,
	COLOR = 3,
	EMITTANCE = 4,
	OBJECT_ID = 5,
	SURFACE_CHARACTERISTIC = 6,
	RANDOM = 7
};

/**
 * This class contains all the OpenCL context information, starts the path tracing execution and returns the result
 */
class CLTracer : public RenderInfoListener
{
private:
	cl_device_id deviceId = nullptr;
	cl_context context = nullptr;
	cl_command_queue commandQueue = nullptr;

	cl_program program = nullptr;
	cl_kernel renderKernel = nullptr;
	cl_kernel clearKernel = nullptr;

	cl_mem image = nullptr;
	cl_mem camera = nullptr;
	cl_mem spheres = nullptr;
	cl_mem lightSpheres = nullptr;
	cl_mem triangles = nullptr;
	cl_mem lightTriangles = nullptr;
	cl_mem sceneInfo = nullptr;

	GLuint textureTargetId = 0;

	size_t localWorkSize[2] = {0, 0};
	size_t globalWorkSize[2] = {0, 0};

	Scene *scene;
	RenderOption option = DEFAULT;

	long renderStartTime = -1;
	int width = 0, height = 0;

	void loadContext();

	void loadDevice();

	void loadCommandQueue();

	void loadProgram(const char *programPath);

	void loadKernel(cl_kernel *kernel, const char *kernelName);

	void setGlobalWorkSize();

	void updateScene();

	void updateRenderTarget();

	void clearImage();

	cl_mem createValidBuffer(void *data, size_t size, cl_int *clError);

public:
	explicit CLTracer(Scene *scene, const size_t localWorkSize[2]);

	~CLTracer();

	bool init(const char *programPath, GLuint textureBufferId);

	void changeScene(Scene *scene);

	void trace();

	void resetRendering();

	void notify() override;

	void notifySizeChanged(int newWidth, int newHeight) override;

	void updateCamera();

	float getFPS();

	void setRenderOption(RenderOption renderOption);

	cl_int iteration = 0;
	int maxSamples = 1000;
};
