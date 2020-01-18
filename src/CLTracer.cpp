//
// Created by Christian Navolskyi on 04.01.20.
//

#include <vector>
#include <iostream>
#include <chrono>

#include <OpenCL/cl_gl.h>
#include <OpenGL/CGLCurrent.h>
#include <GL/glew.h>
#include <OpenCL/opencl.h>
#include <OpenCL/cl.hpp>

#include "CLUtil.h"
#include "CLTracer.h"

long getCurrentTime()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

CLTracer::CLTracer(Scene *scene, const size_t localWorkSize[2]) : scene(scene)
{
	this->localWorkSize = cl::NDRange(localWorkSize[0], localWorkSize[1]);

	auto time = getCurrentTime();

	srand48(time);
	scene->linkUpdateListener(this);
}

bool CLTracer::init(const std::string &kernelDir, GLuint textureBufferId)
{
	cl_int clError;

	loadContext();
	loadDevice();
	loadCommandQueue();

	loadProgram(kernelDir);

	loadKernel(renderKernel, "render");

	this->textureTargetId = textureBufferId;

	camera = cl::Buffer(context, CL_MEM_READ_ONLY, sizeof(Camera), nullptr, &clError);
	sceneInfo = cl::Buffer(context, CL_MEM_READ_ONLY, sizeof(SceneInfo), nullptr, &clError);
	V_RETURN_FALSE_CL(clError, "Failed to allocate space for camera");

	updateScene();

	renderStartTime = getCurrentTime();

	return true;
}

void CLTracer::loadContext()
{
	cl_int clError;

	CGLContextObj cglCurrentContext = CGLGetCurrentContext();
	CGLShareGroupObj cglShareGroup = CGLGetShareGroup(cglCurrentContext);
	cl_context_properties props[] =
			{
					CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, (cl_context_properties) cglShareGroup,
					0};

	context = cl::Context(CL_DEVICE_TYPE_GPU, props, [](const char *errinfo, const void *private_info, size_t cb, void *user_data) -> void
	{
		std::cout << errinfo << std::endl;
	}, nullptr, &clError);
	V_EXIT_CL(clError, "Failed to create shared OpenCL context.");
}

void CLTracer::loadDevice()
{
	cl_int clError;

	std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>(&clError);

	if (devices.empty())
	{
		std::cout << "No OpenCL devices found" << std::endl;
		exit(-1);
	}

	std::cout << "Found " << devices.size() << " OpenCL devices" << std::endl;

	size_t globalMemSize = 0;
	int index = -1;

	for (int i = 0; i < devices.size(); i++)
	{
		size_t memorySize = devices[i].getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>();

		std::cout << "\t" << i << ": " << std::endl;
		std::cout << "\t\tDevice Name: " << devices[i].getInfo<CL_DEVICE_NAME>() << std::endl;
		std::cout << "\t\tDevice Available: " << devices[i].getInfo<CL_DEVICE_AVAILABLE>() << std::endl;
		std::cout << "\t\tDevice Compiler Available: " << devices[i].getInfo<CL_DEVICE_COMPILER_AVAILABLE>() << std::endl;
		std::cout << "\t\tDevice Type: " << devices[i].getInfo<CL_DEVICE_TYPE>() << std::endl;
		std::cout << "\t\tDevice Global Memory Size: " << memorySize << std::endl;
		std::cout << "\t\tDevice Maximum Memory Allocation Size: " << devices[i].getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>() << std::endl;

		if (memorySize > globalMemSize)
		{
			index = i;
			device = devices[i];
		}
	}

	if (index == -1)
	{
		std::cout << "No OpenCL device found - exiting application" << std::endl;
		exit(-1);
	}
	else
	{
		std::cout << "Selected Device " << index << std::endl;
	}
}

void CLTracer::loadCommandQueue()
{
	cl_int clError;

	commandQueue = cl::CommandQueue(context, device, 0, &clError);
	V_RETURN_CL(clError, "Failed to create command queue");
}

void CLTracer::loadProgram(const std::string &kernelDirectory)
{
	cl_int clError;
	std::string mainSourceCode;
	std::vector<std::string> includeSourceCodes;
	std::vector<cl_program> includePrograms;

	CLUtil::LoadProgramSourceToMemory(kernelDirectory + mainProgramName, mainSourceCode);

	for (const auto &name: includeNames)
	{
		std::string tmpSourceCode;

		CLUtil::LoadProgramSourceToMemory(kernelDirectory + name, tmpSourceCode);

		includeSourceCodes.push_back(tmpSourceCode);
	}

	const char *rawMainSource = mainSourceCode.c_str();
	size_t mainSourceLength = mainSourceCode.size();

	program = clCreateProgramWithSource(context(), 1, &rawMainSource, &mainSourceLength, &clError);
	V_RETURN_CL(clError, "Failed to create program from main source");

	for (const auto &includeSourceCode: includeSourceCodes)
	{
		const char *tmpSource = includeSourceCode.c_str();
		size_t tmpLength = includeSourceCode.size();

		cl_program tmpProgram = clCreateProgramWithSource(context(), 1, &tmpSource, &tmpLength, &clError);

		includePrograms.push_back(tmpProgram);
	}
	V_RETURN_CL(clError, "Failed to create source from include sources");

	cl_device_id deviceId = device();
	clError = clCompileProgram(program(), 1, &deviceId, "", numIncludeFiles, &includePrograms[0], includeNames, nullptr, nullptr);
	V_RETURN_CL(clError, "Failed to compile program");

	std::vector<cl::Device> devices;
	devices.push_back(device);

	clError = program.build(devices, "-I ../src/kernels/", [](cl_program buildProgram, void *data)
	{
		cl::Program buildCLProgram(buildProgram);
		auto *buildDevice = (cl::Device *) data;

		std::vector<cl::Device> programDevices = buildCLProgram.getInfo<CL_PROGRAM_DEVICES>();

		std::cout << "Build log: " << buildCLProgram.getBuildInfo<CL_PROGRAM_BUILD_LOG>(*buildDevice) << std::endl
				  << "Build devices (" << programDevices.size() << ") name: " << programDevices[0].getInfo<CL_DEVICE_NAME>() << std::endl;
	}, &device);
	V_EXIT_CL(clError, "Failed to build program");
}

void CLTracer::loadKernel(cl::Kernel kernel, const char *kernelName)
{
	cl_int clError;

	kernel = cl::Kernel(program, kernelName, &clError);

	std::string errorMessage = "Failed to create kernel: ";
	errorMessage += kernelName;
	V_RETURN_CL(clError, errorMessage);
}

void CLTracer::trace()
{
	if (iteration >= maxSamples)
	{
		return;
	}

	if (option == RANDOM)
	{
		iteration = 0;
	}

	glFinish();

	auto randomNumberSeed = (float) (rand() / (double) RAND_MAX);

	cl_int clError;

	clError = clEnqueueAcquireGLObjects(commandQueue(), 1, &image(), 0, nullptr, nullptr);
	V_RETURN_CL(clError, "Failed to write data to OpenCL");

	clError = renderKernel.setArg(0, image);
	clError |= renderKernel.setArg(1, spheres);
	clError |= renderKernel.setArg(2, lightSpheres);
	clError |= renderKernel.setArg(3, triangles);
	clError |= renderKernel.setArg(4, lightTriangles);
	clError |= renderKernel.setArg(5, sceneInfo);
	clError |= renderKernel.setArg(6, camera);
	clError |= renderKernel.setArg(7, iteration);
	clError |= renderKernel.setArg(8, randomNumberSeed);
	clError |= renderKernel.setArg(9, option);

	commandQueue.enqueueNDRangeKernel(renderKernel, cl::NDRange(0, 0), globalWorkSize, localWorkSize);
	V_RETURN_CL(clError, "Failed to enqueue trace kernel task");

	clError = clEnqueueReleaseGLObjects(commandQueue(), 1, &image(), 0, nullptr, nullptr);
	V_RETURN_CL(clError, "Failed to release texture from OpenCL");

	iteration++;
	commandQueue.finish();
}

cl_int CLTracer::setValidKernelArg(cl_uint position, cl_mem data)
{
	if (data)
	{
		return renderKernel.setArg(position, &data);
	}
	else
	{
		return renderKernel.setArg(position, nullptr);
	}
}

void CLTracer::updateScene()
{
	glFinish();

	cl_int clError;

	Sphere *sphereData = scene->getSphereData();
	LightSphere *lightSphereData = scene->getLightSphereData();
	Triangle *triangleData = scene->getTriangleData();
	LightTriangle *lightTriangleData = scene->getLightTriangleData();

	spheres = createValidBuffer(sphereData, scene->getSphereSize(), &clError);
	lightSpheres = createValidBuffer(lightSphereData, scene->getLightSphereSize(), &clError);
	triangles = createValidBuffer(triangleData, scene->getTriangleSize(), &clError);
	lightTriangles = createValidBuffer(lightTriangleData, scene->getLightTriangleSize(), &clError);
	clError |= commandQueue.enqueueWriteBuffer(sceneInfo, CL_FALSE, 0, sizeof(SceneInfo), scene->getSceneInfo());
	V_RETURN_CL(clError, "Failed to initialize buffers for scene info");

	commandQueue.finish();

	glm::ivec2 resolution = scene->getResolution();
	notifySizeChanged(resolution.x, resolution.y);
}

cl::Buffer CLTracer::createValidBuffer(void *data, size_t size, cl_int *clError)
{
	if (data)
	{
		return cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, size, data, clError);
	}
	else
	{
		return cl::Buffer();
	}
}

void CLTracer::changeScene(Scene *scene)
{
	this->scene = scene;

	updateScene();
	resetRendering();
}

void CLTracer::resetRendering()
{
	iteration = 0;
	renderStartTime = getCurrentTime();
}

void CLTracer::notify()
{
	updateCamera();
	resetRendering();
}

void CLTracer::notifySizeChanged(int newWidth, int newHeight)
{
	width = newWidth;
	height = newHeight;

	size.clear();
	size.push_back(width);
	size.push_back(height);

	setGlobalWorkSize();
	updateRenderTarget();
	updateCamera();
	resetRendering();
}

void CLTracer::setGlobalWorkSize()
{
	globalWorkSize = CLUtil::GetGlobalWorkSize(size, localWorkSize);
}

void CLTracer::updateRenderTarget()
{
	cl_int clError;

	image = cl::BufferGL(context, CL_MEM_READ_WRITE, textureTargetId, &clError);
	V_RETURN_CL(clError, "Failed to create link to shared resources");
}

void CLTracer::updateCamera()
{
	glFinish();

	cl_int clError;

	Camera renderCamera = scene->getRenderCamera();
	clError = commandQueue.enqueueWriteBuffer(camera, CL_FALSE, 0, sizeof(Camera), &renderCamera);
	V_RETURN_CL(clError, "Failed to load scene and/or camera to OpenCL");

	commandQueue.finish();
}

float CLTracer::getFPS()
{
	long currentTime = getCurrentTime();
	float timeDiff = currentTime - renderStartTime;

	return iteration * 1000.f / timeDiff;
}

void CLTracer::setRenderOption(RenderOption renderOption)
{
	option = renderOption;
	resetRendering();
}
