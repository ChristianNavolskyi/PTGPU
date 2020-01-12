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

#include "CLUtil.h"
#include "CLTracer.h"

void printImageData(cl_command_queue commandQueue, cl_mem image, int width, int height)
{
	size_t values = 4;
	float *imageData = (float *) malloc(sizeof(float) * values * width * height);

	clFinish(commandQueue);

//	clEnqueueAcquireGLObjects(commandQueue, 1, &image, 0, nullptr, nullptr);
	clEnqueueReadBuffer(commandQueue, image, CL_TRUE, 0, sizeof(float) * values * width * height, imageData, 0, nullptr, nullptr);
//	clEnqueueReleaseGLObjects(commandQueue, 1, &image, 0, nullptr, nullptr);

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			std::cout << "("
					  << imageData[y * width * values + x * values] << ", "
					  << imageData[y * width * values + x * values + 1] << ", "
					  << imageData[y * width * values + x * values + 2] << ") ";
		}
		std::cout << std::endl;
	}

	free(imageData);
}

CLTracer::CLTracer(Scene *scene, const size_t localWorkSize[2]) : scene(scene)
{
	this->localWorkSize[0] = localWorkSize[0];
	this->localWorkSize[1] = localWorkSize[1];

	auto time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	srand48(time);
	scene->linkUpdateListener(this);
}

CLTracer::~CLTracer()
{
	clReleaseKernel(renderKernel);
	clReleaseKernel(clearKernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(commandQueue);
	clReleaseContext(context);
	clReleaseDevice(deviceId);
}

bool CLTracer::init(const char *programPath, GLuint textureBufferId)
{
	loadContext();
	loadDevice();
	loadCommandQueue();

	loadProgram(programPath);
	loadKernel(&renderKernel, "render");
	loadKernel(&clearKernel, "clearImage");

	this->textureTargetId = textureBufferId;

	cl_int clError;

	camera = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(Camera), nullptr, &clError);
	V_RETURN_FALSE_CL(clError, "Failed to allocate space for camera");

	updateScene();

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

	context = clCreateContext(props, 0, nullptr, [](const char *errinfo, const void *private_info, size_t cb, void *user_data) -> void
	{
		/* context-creation and runtime error handler */
		std::cout << "Context error: " << errinfo << std::endl;
	}, nullptr, &clError);
	V_RETURN_CL(clError, "Failed to create OpenCL context.");
}

void CLTracer::loadDevice()
{
	cl_int clError;
	size_t returnedSize;
	cl_device_id deviceIds[16];

	clError = clGetContextInfo(context, CL_CONTEXT_DEVICES, sizeof(cl_device_id), (void *) deviceIds, &returnedSize);
	V_RETURN_CL(clError, "Failed to get devices from context");

	int deviceCount = returnedSize / sizeof(cl_device_id);
	bool deviceFound = false;

	std::cout << "Got " << deviceCount << " devices: " << std::endl;

	for (int i = 0; i < deviceCount; i++)
	{
		cl_device_type deviceType;
		int nameSize = 512;
		char deviceName[nameSize];
		size_t actualSize;

		clError = clGetDeviceInfo(deviceIds[i], CL_DEVICE_NAME, sizeof(char) * nameSize, deviceName, &actualSize);
		V_RETURN_CL(clError, "Failed to read device name");
		std::cout << "\tDevice[" << i << "]: Name - " << deviceName << std::endl;

		clError = clGetDeviceInfo(deviceIds[i], CL_DEVICE_TYPE, sizeof(cl_device_type), &deviceType, &actualSize);
		V_RETURN_CL(clError, "Failed to read device type");
		std::cout << "\tDevice[" << i << "]: Device Type - " << deviceType << std::endl;

		if (deviceType == CL_DEVICE_TYPE_GPU)
		{
			std::cout << "\tSelecting device " << i << std::endl;
			deviceId = deviceIds[i];
			deviceFound = true;
			break;
		}
	}

	if (!deviceFound)
	{
		std::cout << "No OpenCL device found - exiting application" << std::endl;
		exit(-1);
	}
}

void CLTracer::loadCommandQueue()
{
	cl_int clError;

	commandQueue = clCreateCommandQueue(context, deviceId, 0, &clError);
	V_RETURN_CL(clError, "Failed to create command queue");
}

void CLTracer::loadProgram(const char *programPath)
{
	std::string programCode;

	CLUtil::LoadProgramSourceToMemory(programPath, programCode);
	program = CLUtil::BuildCLProgramFromMemory(deviceId, context, programCode);

	if (program == nullptr)
	{
		std::cout << "Failed to load OpenCL program" << std::endl;
		exit(-1);
	}
}

void CLTracer::loadKernel(cl_kernel *kernel, const char *kernelName)
{
	cl_int clError;
	*kernel = clCreateKernel(program, kernelName, &clError);

	std::string errorMessage = "Failed to create kernel: ";
	errorMessage += kernelName;
	V_RETURN_CL(clError, errorMessage);
}

void CLTracer::clearImage(float *imageData)
{
	glFinish();

	cl_int clError;

	size_t imageSize = sizeof(float) * 4 * width * height;

	clError = clEnqueueAcquireGLObjects(commandQueue, 1, &image, 0, nullptr, nullptr);
	V_RETURN_CL(clError, "Failed to acquire texture to clear image");
//
//	clError = clEnqueueWriteBuffer(commandQueue, image, GL_FALSE, 0, imageSize, imageData, 0, nullptr, nullptr);
//	V_RETURN_CL(clError, "Failed to write image to buffer");

	clError = clSetKernelArg(clearKernel, 0, sizeof(cl_mem), (void *) &image);
	clError |= clSetKernelArg(clearKernel, 1, sizeof(cl_int), &width);
	clError |= clSetKernelArg(clearKernel, 2, sizeof(cl_int), &height);
	V_RETURN_CL(clError, "Failed to set args to clear kernels");

	clError = clEnqueueNDRangeKernel(commandQueue, clearKernel, 2, nullptr, globalWorkSize, localWorkSize, 0, nullptr, nullptr);
	V_RETURN_CL(clError, "Failed to execute clear kernel");

	clError = clEnqueueReleaseGLObjects(commandQueue, 1, &image, 0, nullptr, nullptr);
	V_RETURN_CL(clError, "Failed to release texture after clearing image");
//
//	clError = clEnqueueReadBuffer(commandQueue, image, GL_FALSE, 0, imageSize, imageData, 0, nullptr, nullptr);
//	V_RETURN_CL(clError, "Failed to read image to buffer");

	clFinish(commandQueue);

//	printImageData(commandQueue, image, width, height);
}

void CLTracer::trace(float *imageData)
{
	glFinish();

	auto randomNumberSeed = (float) (rand() / (double) RAND_MAX);
	size_t imageSize = sizeof(float) * 4 * width * height;

	cl_int clError;

	clError = clEnqueueAcquireGLObjects(commandQueue, 1, &image, 0, nullptr, nullptr);
	V_RETURN_CL(clError, "Failed to write data to OpenCL");
//
//	clError = clEnqueueWriteBuffer(commandQueue, image, GL_FALSE, 0, imageSize, imageData, 0, nullptr, nullptr);
//	V_RETURN_CL(clError, "Failed to write image to buffer");

	clError = clSetKernelArg(renderKernel, 0, sizeof(cl_mem), (void *) &image);
	clError |= clSetKernelArg(renderKernel, 1, sizeof(cl_mem), (void *) &spheres);
	clError |= clSetKernelArg(renderKernel, 2, sizeof(cl_int), (void *) &sphereCount);
	clError |= clSetKernelArg(renderKernel, 3, sizeof(cl_mem), (void *) &camera);
	clError |= clSetKernelArg(renderKernel, 4, sizeof(cl_int), (void *) &iteration);
	clError |= clSetKernelArg(renderKernel, 5, sizeof(cl_float), (void *) &randomNumberSeed);
	V_RETURN_CL(clError, "Failed to set trace kernel arguments");

	clError = clEnqueueNDRangeKernel(commandQueue, renderKernel, 2, nullptr, globalWorkSize, localWorkSize, 0, nullptr, nullptr);
	V_RETURN_CL(clError, "Failed to enqueue trace kernel task");

	clError = clEnqueueReleaseGLObjects(commandQueue, 1, &image, 0, nullptr, nullptr);
	V_RETURN_CL(clError, "Failed to release texture from OpenCL");
//
//	clError = clEnqueueReadBuffer(commandQueue, image, GL_FALSE, 0, imageSize, imageData, 0, nullptr, nullptr);
//	V_RETURN_CL(clError, "Failed to read image to buffer");

	iteration++;
	clFinish(commandQueue);
}

void CLTracer::updateScene()
{
	cl_int clError;

	spheres = clCreateBuffer(context, CL_MEM_READ_ONLY, scene->getSphereSize(), nullptr, &clError);
	V_RETURN_CL(clError, "Failed to allocate space for camera");

	sphereCount = scene->getSphereCount();

	glFinish();

	clError = clEnqueueWriteBuffer(commandQueue, spheres, CL_FALSE, 0, scene->getSphereSize(), scene->getSphereData(), 0, nullptr, nullptr);
	V_RETURN_CL(clError, "Failed to load scene to OpenCL");

	clFinish(commandQueue);

	updateCamera();

	glm::ivec2 resolution = scene->getResolution();
	notifySizeChanged(resolution.x, resolution.y);
}

void CLTracer::changeScene(Scene *scene)
{
	this->scene = scene;

	updateScene();
	resetRendering();
}

void CLTracer::resetRendering()
{
//	clearImage(nullptr);

	iteration = 0;
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

	setGlobalWorkSize();
	updateRenderTarget();
	resetRendering();
}

void CLTracer::setGlobalWorkSize()
{
	globalWorkSize[0] = CLUtil::GetGlobalWorkSize(width, localWorkSize[0]);
	globalWorkSize[1] = CLUtil::GetGlobalWorkSize(height, localWorkSize[1]);
}

void CLTracer::updateRenderTarget()
{
	cl_int clError;

//	image = clCreateFromGLTexture(context, CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, textureTargetId, &clError);
	image = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, textureTargetId, &clError);
//	image = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cl_float4) * width * height, nullptr, &clError);
	V_RETURN_CL(clError, "Failed to create link to shared resources");

//	GLenum textureTarget;
//	clError = clGetGLTextureInfo(image, CL_GL_TEXTURE_TARGET, sizeof(GLenum), &textureTarget, nullptr);
//	V_RETURN_CL(clError, "Failed to get opengl-opencl texture target");
//
//	std::cout << "texture target: " << textureTarget << std::endl;

//	printImageData(commandQueue, image, width, height);
}

void CLTracer::updateCamera()
{
	glFinish();

	cl_int clError;

	Camera renderCamera = scene->getRenderCamera();
	clError = clEnqueueWriteBuffer(commandQueue, camera, CL_TRUE, 0, sizeof(Camera), &renderCamera, 0, nullptr, nullptr);
	V_RETURN_CL(clError, "Failed to load scene and/or camera to OpenCL");

	clFinish(commandQueue);
}
