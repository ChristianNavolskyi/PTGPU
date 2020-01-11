//
// Created by Christian Navolskyi on 04.01.20.
//

#include <vector>
#include <iostream>
#include <chrono>

#include <GL/glew.h>
#include <OpenCL/opencl.h>
#include <OpenGL/CGLCurrent.h>

#include "CLUtil.h"
#include "CLTracer.h"

CLTracer::CLTracer(Scene scene, const size_t localWorkSize[2]) :
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

bool CLTracer::init(const char *programPath, GLuint textureBufferId)
{
	loadContext();
	loadDevice();
	loadCommandQueue();

	loadProgram(programPath);
	loadKernel(&renderKernel, "render");
	loadKernel(&clearKernel, "clearImage");
	loadKernel(&initializeRenderPlaneKernel, "initializeRenderPlane");

	linkOpenGLResources(textureBufferId);

	allocateFixedCLBuffers();

	initScene();

	return true;
}

void CLTracer::loadContext()
{
	std::cout << "Creating OpenCL shared context..." << std::endl;

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

	std::cout << "Created OpenCL context successfully." << std::endl;
}

void CLTracer::loadDevice()
{
	std::cout << "Setting up OpenCL device..." << std::endl;

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
	else
	{
		std::cout << "Setup OpenCL device successfully" << std::endl;
	}
}

void CLTracer::loadCommandQueue()
{
	std::cout << "Creating OpenCL command queue..." << std::endl;

	cl_int clError;

	commandQueue = clCreateCommandQueue(context, deviceId, 0, &clError);
	V_RETURN_CL(clError, "Failed to create command queue");

	std::cout << "Created OpenCL command queue successfully." << std::endl;
}

void CLTracer::loadProgram(const char *programPath)
{
	std::cout << "Loading OpenCL program..." << std::endl;

	std::string programCode;

	CLUtil::LoadProgramSourceToMemory(programPath, programCode);
	program = CLUtil::BuildCLProgramFromMemory(deviceId, context, programCode);

	if (program == nullptr)
	{
		std::cout << "Failed to load OpenCL program" << std::endl;
		exit(-1);
	}
	else
	{
		std::cout << "Loaded OpenCL program successfully" << std::endl;
	}
}

void CLTracer::loadKernel(cl_kernel *kernel, const char *kernelName)
{
	std::cout << "Loading " << kernelName << " kernel..." << std::endl;

	cl_int clError;
	*kernel = clCreateKernel(program, kernelName, &clError);

	std::string errorMessage = "Failed to create kernel: ";
	errorMessage += kernelName;
	V_RETURN_CL(clError, errorMessage);

	std::cout << "Loaded " << kernelName << " kernel successfully" << std::endl;
}

void CLTracer::clearImage()
{
	glFinish();

	cl_int clError;

	clError = clEnqueueNDRangeKernel(commandQueue, clearKernel, 2, nullptr, globalWorkSize, localWorkSize, 0, nullptr, nullptr);
	V_RETURN_CL(clError, "Failed to execute clear kernel");

	clFinish(commandQueue);
}

void CLTracer::initializeRenderPlane()
{
	glFinish();

	cl_int clError;

	clError = clEnqueueNDRangeKernel(commandQueue, initializeRenderPlaneKernel, 2, nullptr, globalWorkSize, localWorkSize, 0, nullptr, nullptr);
	V_RETURN_CL(clError, "Failed to execute initialization kernel");

	clFinish(commandQueue);
}

void CLTracer::trace()
{
	glFinish();

	auto randomNumberSeed = (float) (rand() / (double) RAND_MAX);

	cl_int clError;

	clError = clEnqueueAcquireGLObjects(commandQueue, 1, &image, 0, nullptr, nullptr);
	V_RETURN_CL(clError, "Failed to write data to OpenCL");

	clError = clSetKernelArg(renderKernel, 4, sizeof(cl_int), (void *) &iteration);
	clError |= clSetKernelArg(renderKernel, 5, sizeof(cl_float), (void *) &randomNumberSeed);
	V_RETURN_CL(clError, "Failed to set kernel arguments");

	clError = clEnqueueNDRangeKernel(commandQueue, renderKernel, 2, nullptr, globalWorkSize, localWorkSize, 0, nullptr, nullptr);
	V_RETURN_CL(clError, "Failed to enqueue kernel task");

	clError = clEnqueueReleaseGLObjects(commandQueue, 1, &image, 0, nullptr, nullptr);
	V_RETURN_CL(clError, "Failed to read texture from OpenCL");

	iteration++;
	clFinish(commandQueue);
}

void CLTracer::initScene()
{
	std::cout << "Creating OpenCL buffer for spheres..." << std::endl;

	cl_int clError;

	spheres = clCreateBuffer(context, CL_MEM_READ_ONLY, scene.getSphereSize(), nullptr, &clError);
	V_RETURN_CL(clError, "Failed to allocate space for camera");

	std::cout << "Created OpenCL buffer for spheres successfully" << std::endl;

	sphereCount = scene.getSphereCount();

	clError = clSetKernelArg(renderKernel, 1, sizeof(cl_mem), (void *) &spheres);
	clError |= clSetKernelArg(renderKernel, 2, sizeof(cl_int), (void *) &sphereCount);
	V_RETURN_CL(clError, "Failed to set kernel args for sphere data");

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
//	initializeRenderPlane();

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

	setSizeArgs();

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
	std::cout << "Creating OpenCL buffers from OpenGL resources..." << std::endl;

	glFinish();

	cl_int clError;

	image = clCreateFromGLTexture(context, CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, textureTargetId, &clError);
	V_RETURN_CL(clError, "Failed to create link to shared resources");

	clError = clSetKernelArg(renderKernel, 0, sizeof(cl_mem), (void *) &image);
	clError |= clSetKernelArg(clearKernel, 0, sizeof(cl_mem), (void *) &image);
	V_RETURN_CL(clError, "Failed to set kernel args from image color and vertex buffer");

	clFinish(commandQueue);

	std::cout << "Created OpenCL buffers from OpenGL resources successfully" << std::endl;
}

void CLTracer::allocateFixedCLBuffers()
{
	std::cout << "Creating OpenCL buffer for camera..." << std::endl;

	cl_int clError;

	camera = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(Camera), nullptr, &clError);
	V_RETURN_CL(clError, "Failed to allocate space for camera");

	clError = clSetKernelArg(renderKernel, 3, sizeof(cl_mem), (void *) &camera);
	V_RETURN_CL(clError, "Failed to set kernel arg for camera");

	std::cout << "Created OpenCL buffer for camera successfully" << std::endl;
}

void CLTracer::writeSceneBuffer()
{
	std::cout << "Writing spheres to OpenCL buffer..." << std::endl;

	glFinish();

	cl_int clError;

	clError = clEnqueueWriteBuffer(commandQueue, spheres, CL_FALSE, 0, sizeof(scene.getSphereSize()), scene.getSphereData(), 0, nullptr, nullptr);
	V_RETURN_CL(clError, "Failed to load scene to OpenCL");

	clFinish(commandQueue);

	std::cout << "Wrote spheres to OpenCL buffer successfully" << std::endl;
}

void CLTracer::writeCameraBuffer()
{
	std::cout << "Writing camera to OpenCL buffer..." << std::endl;

	glFinish();

	cl_int clError;

	Camera renderCamera = scene.getRenderCamera();
	clError = clEnqueueWriteBuffer(commandQueue, camera, CL_FALSE, 0, sizeof(Camera), &renderCamera, 0, nullptr, nullptr);
	V_RETURN_CL(clError, "Failed to load camera to OpenCL");

	clFinish(commandQueue);

	std::cout << "Wrote camera to OpenCL buffer successfully" << std::endl;
}

void CLTracer::linkOpenGLResources(GLuint textureBufferId)
{
	this->textureTargetId = textureBufferId;

	initializeRenderTargets();
}

void CLTracer::setSizeArgs()
{
	cl_int clError;

	clError = clSetKernelArg(clearKernel, 1, sizeof(cl_int), &width);
	clError |= clSetKernelArg(clearKernel, 2, sizeof(cl_int), &height);
	clError |= clSetKernelArg(initializeRenderPlaneKernel, 1, sizeof(cl_int), &width);
	clError |= clSetKernelArg(initializeRenderPlaneKernel, 2, sizeof(cl_int), &height);
	V_RETURN_CL(clError, "Failed to set size args to kernels");
}
