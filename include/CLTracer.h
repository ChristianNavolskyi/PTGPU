//
// Created by Christian Navolskyi on 04.01.20.
//

#pragma once

#include <OpenCL/cl.hpp>
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
	SPHERE_ID = 5,
	RANDOM = 6
};

/**
 * This class contains all the OpenCL context information, starts the path tracing execution and returns the result
 */
class CLTracer : public RenderInfoListener
{
private:
	cl::Device device;
	cl::Context context;
	cl::CommandQueue commandQueue;

	cl::Kernel renderKernel;
	cl::Program program;

	cl::BufferGL image;
	cl::Buffer camera;
	cl::Buffer spheres;
	cl::Buffer lightSpheres;
	cl::Buffer triangles;
	cl::Buffer lightTriangles;
	cl::Buffer sceneInfo;

	GLuint textureTargetId = 0;

	cl::NDRange localWorkSize;
	cl::NDRange globalWorkSize;

	Scene *scene;
	RenderOption option = DEFAULT;

	static const cl_uint numIncludeFiles = 6;
	std::string mainProgramName = "pathtracer.cl";
	const char *includeNames[numIncludeFiles] = {"math.cl", "ray.cl", "scene.cl", "camera.cl", "spheres.cl", "triangles.cl"};

//	static const cl_uint numIncludeFiles = 1;
//	std::string mainProgramName = "main.cl";
//	const char *includeNames[numIncludeFiles] = {"other.cl"};


	long renderStartTime = -1;
	std::vector<size_t> size;
	int width = 0, height = 0;

	void loadContext();

	void loadDevice();

	void loadCommandQueue();

	void loadProgram(const std::string &kernelDirectory);

	void loadKernel(cl::Kernel kernel, const char *kernelName);

	void setGlobalWorkSize();

	void updateScene();

	void updateRenderTarget();

	cl::Buffer createValidBuffer(void *data, size_t size, cl_int *clError);

	cl_int setValidKernelArg(cl_uint position, cl_mem data);

public:
	explicit CLTracer(Scene *scene, const size_t localWorkSize[2]);

	~CLTracer() override = default;

	bool init(const std::string &kernelDir, GLuint textureBufferId);

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
