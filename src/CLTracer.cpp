//
// Created by Christian Navolskyi on 04.01.20.
//

#include <vector>
#include <iostream>
#include <chrono>

#include <OpenCL/cl_gl.h>
#include <OpenGL/CGLCurrent.h>
#include <GL/glew.h>

#include "CLUtil.h"
#include "CLTracer.h"

CLTracer::CLTracer(Scene scene, const size_t localWorkSize[2]) : platformId(nullptr),
																 commandQueue(nullptr),
																 deviceId(nullptr),
																 context(nullptr),
																 renderKernel(nullptr),
																 clearKernel(nullptr),
																 initializeRenderPlaneKernel(nullptr),
																 program(nullptr), scene(scene)
{
	this->localWorkSize[0] = localWorkSize[0];
	this->localWorkSize[1] = localWorkSize[1];

	auto time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	srand48(time);
	scene.linkUpdateListener(this);
}

CLTracer::~CLTracer()
{
	clReleaseKernel(renderKernel);
	clReleaseKernel(clearKernel);
	clReleaseKernel(initializeRenderPlaneKernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(commandQueue);
	clReleaseContext(context);
	clReleaseDevice(deviceId);
}

bool CLTracer::init(const char *programPath)
{
	loadPlatformAndDevice();
	loadContext();
	loadCommandQueue();

	loadProgram(programPath);
	loadKernel(&renderKernel, "render");
	loadKernel(&clearKernel, "clearImage");
	loadKernel(&initializeRenderPlaneKernel, "initializeRenderPlane");

	allocateFixedCLBuffers();

	initScene();

	return true;
}

void CLTracer::loadPlatformAndDevice()
{
	std::vector<cl_platform_id> platformIds;
	const cl_uint maxPlatforms = 2;
	platformIds.resize(maxPlatforms);
	cl_uint countPlatforms;

	clGetPlatformIDs(maxPlatforms, &platformIds[0], &countPlatforms);
	platformIds.resize(countPlatforms);

	// 2. find all available GPU devices
	std::vector<cl_device_id> deviceIds;
	const int maxDevices = 16;
	deviceIds.resize(maxDevices);
	int countAllDevices = 0;

	// Searching for the graphics device with the most dedicated video memory.
	cl_ulong maxGlobalMemorySize = 0;
	cl_device_id bestDeviceId = nullptr;

	cl_device_type deviceType = CL_DEVICE_TYPE_GPU;

	for (auto &platformId : platformIds)
	{
		// Getting the available devices.
		cl_uint countDevices;
		auto res = clGetDeviceIDs(platformId, deviceType, maxDevices, &deviceIds[countAllDevices], &countDevices);
		if (res != CL_SUCCESS) // Maybe there are no GPU devices and some poor implementation doesn't set count devices to zero and return CL_DEVICE_NOT_FOUND.
		{
			char buffer[1024];
			clGetPlatformInfo(platformId, CL_PLATFORM_NAME, 1024, buffer, nullptr);
			std::cout << "[WARNING]: clGetDeviceIDs() failed. Error type: " << CLUtil::GetCLErrorString(res) << ", Platform name: " << buffer << "!" << std::endl;
			continue;
		}

		for (size_t j = 0; j < countDevices; j++)
		{
			cl_device_id currentDeviceId = deviceIds[countAllDevices + j];
			cl_ulong globalMemorySize;
			cl_bool isUsingUnifiedMemory;
			clGetDeviceInfo(currentDeviceId, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &globalMemorySize, nullptr);
			clGetDeviceInfo(currentDeviceId, CL_DEVICE_HOST_UNIFIED_MEMORY, sizeof(cl_bool), &isUsingUnifiedMemory, nullptr);

			if (!isUsingUnifiedMemory && globalMemorySize > maxGlobalMemorySize)
			{
				bestDeviceId = currentDeviceId;
				maxGlobalMemorySize = globalMemorySize;
			}
		}
		countAllDevices += countDevices;
	}
	deviceIds.resize(countAllDevices);

	if (countAllDevices == 0)
	{
		std::cout << "No device of the selected type with OpenCL support was found.";
		exit(-1);
	}

	// No discrete graphics device was found: falling back to the first found device.
	if (bestDeviceId == nullptr)
	{
		bestDeviceId = deviceIds[0];
	}

	// print information about chosen platform and device
	deviceId = bestDeviceId;
	clGetDeviceInfo(deviceId, CL_DEVICE_PLATFORM, sizeof(cl_platform_id), &platformId, nullptr);
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

	context = clCreateContext(props, 1, &deviceId, nullptr, nullptr, &clError);
	V_RETURN_CL(clError, "Failed to create OpenCL context.");

	std::cout << "Created OpenCL context successfully." << std::endl;
}

void CLTracer::loadCommandQueue()
{
	cl_int clError;
	commandQueue = clCreateCommandQueue(context, deviceId, 0, &clError);
	V_RETURN_CL(clError, "Failed to create the command queue in the context.");
	std::cout << "Created command queue successfully." << std::endl;
}

void CLTracer::loadProgram(const char *programPath)
{
	//load and compile kernels
	std::string programCode;

	CLUtil::LoadProgramSourceToMemory(programPath, programCode);
	program = CLUtil::BuildCLProgramFromMemory(deviceId, context, programCode);

	if (program == nullptr)
		exit(-1);
}

void CLTracer::loadKernel(cl_kernel *kernel, const char *kernelName)
{
	cl_int clError;
	*kernel = clCreateKernel(program, kernelName, &clError);

	std::string errorMessage = "Failed to create kernel: ";
	errorMessage += kernelName;
	V_RETURN_CL(clError, errorMessage);
}

void CLTracer::clearImage()
{
	glFinish();


	clFinish(commandQueue);
}

void CLTracer::initializeRenderPlane()
{
	glFinish();


	clFinish(commandQueue);
}

void CLTracer::trace(float *imageData)
{
	glFinish();

	size_t textureSize = sizeof(float) * 3 * width * height;
	auto randomNumberSeed = (float) (rand() / (double) RAND_MAX);

	cl_int clError;

	clError = clEnqueueWriteBuffer(commandQueue, image, CL_FALSE, 0, textureSize, imageData, 0, nullptr, nullptr);
	V_RETURN_CL(clError, "Failed to write data to OpenCL");

	clError = clSetKernelArg(renderKernel, 0, sizeof(cl_mem), (void *) &image);
	clError |= clSetKernelArg(renderKernel, 1, sizeof(cl_mem), (void *) &spheres);
	clError |= clSetKernelArg(renderKernel, 2, sizeof(cl_int), (void *) &sphereCount);
	clError |= clSetKernelArg(renderKernel, 3, sizeof(cl_mem), (void *) &camera);
	clError |= clSetKernelArg(renderKernel, 4, sizeof(cl_int), (void *) &iteration);
	clError |= clSetKernelArg(renderKernel, 5, sizeof(cl_float), (void *) &randomNumberSeed);
	V_RETURN_CL(clError, "Failed to set kernel arguments");

	clError = clEnqueueNDRangeKernel(commandQueue, renderKernel, 2, nullptr, globalWorkSize, localWorkSize, 0, nullptr, nullptr);
	V_RETURN_CL(clError, "Failed to enqueue kernel task");

	clError = clEnqueueReadBuffer(commandQueue, image, CL_FALSE, 0, textureSize, imageData, 0, nullptr, nullptr);
	V_RETURN_CL(clError, "Failed to read texture from OpenCL");

	iteration++;
	clFinish(commandQueue);
}

void CLTracer::linkGLRenderTarget(GLenum textureTarget, GLuint textureId)
{
	cl_int clError;

	image = clCreateFromGLTexture(context, CL_MEM_READ_WRITE, textureTarget, 0, textureId, &clError);
	V_RETURN_CL(clError, "Failed to link GLTexture to CLMem");
}

void CLTracer::initScene()
{
	cl_int clError;

	spheres = clCreateBuffer(context, CL_MEM_READ_ONLY, scene.getSphereSize(), nullptr, &clError);
	V_RETURN_CL(clError, "Failed to allocate space for camera");

	sphereCount = scene.getSphereCount();

	writeSceneBuffer();
	writeCameraBuffer();

	glm::ivec2 resolution = scene.getResolution();
	notifySizeChanged(resolution.x, resolution.y);
}

void CLTracer::changeScene(Scene scene)
{
	this->scene = scene;

	initScene();
	resetRendering();
}

void CLTracer::resetRendering()
{
	clearImage();
	initializeRenderPlane();

	iteration = 0;
}

void CLTracer::notify()
{
	// TODO (re-)load camera and spheres into gpu memory.
}

void CLTracer::notifySizeChanged(int newWidth, int newHeight)
{
	width = newWidth;
	height = newHeight;

	setGlobalWorkSize();
	initializeRenderTargets();
	resetRendering();
}

void CLTracer::setGlobalWorkSize()
{
	globalWorkSize[0] = CLUtil::GetGlobalWorkSize(width, localWorkSize[0]);
	globalWorkSize[1] = CLUtil::GetGlobalWorkSize(height, localWorkSize[1]);
}

void CLTracer::initializeRenderTargets()
{
	cl_int clError;

	image = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(float) * 3 * width * height, nullptr, &clError);
	imagePlane = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * 2 * width * height, nullptr, &clError);
	V_RETURN_CL(clError, "Failed to allocate memory for image");
}

void CLTracer::allocateFixedCLBuffers()
{
	cl_int clError;

	camera = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(Camera), nullptr, &clError);
	V_RETURN_CL(clError, "Failed to allocate space for camera");
}

void CLTracer::writeSceneBuffer()
{
	glFinish();

	cl_int clError;

	clError = clEnqueueWriteBuffer(commandQueue, spheres, CL_FALSE, 0, sizeof(scene.getSphereSize()), scene.getSphereData(), 0, nullptr, nullptr);
	V_RETURN_CL(clError, "Failed to load scene to OpenCL");

	clFinish(commandQueue);
}

void CLTracer::writeCameraBuffer()
{
	glFinish();

	cl_int clError;

	Camera renderCamera = scene.getRenderCamera();
	clError = clEnqueueWriteBuffer(commandQueue, camera, CL_FALSE, 0, sizeof(Camera), &renderCamera, 0, nullptr, nullptr);
	V_RETURN_CL(clError, "Failed to load camera to OpenCL");

	clFinish(commandQueue);
}
