//
// Created by Melina Christian Navolskyi on 04.01.20.
//

#include <vector>
#include <iostream>
#include "CLUtil.h"
#include "CLTracer.h"

CLTracer::CLTracer(const size_t localWorkSize[2]) : platformId(nullptr), commandQueue(nullptr), deviceId(nullptr), context(nullptr), kernel(nullptr), program(nullptr)
{
	this->localWorkSize[0] = localWorkSize[0];
	this->localWorkSize[1] = localWorkSize[1];
}

CLTracer::~CLTracer()
{
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(commandQueue);
	clReleaseContext(context);
	clReleaseDevice(deviceId);
}

bool CLTracer::init(const char *programPath, const char *kernelName)
{
	loadPlatformAndDevice();
	loadContext();
	loadCommandQueue();

	loadProgram(programPath);
	loadKernel(kernelName);

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

		const int maxBufferSize = 1024;

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
	context = clCreateContext(nullptr, 1, &deviceId, nullptr, nullptr, &clError);
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

void CLTracer::loadKernel(const char *kernelName)
{
	cl_int clError;
	kernel = clCreateKernel(program, kernelName, &clError);
	V_RETURN_CL(clError, "Failed to create kernel: Reduction_InterleavedAddressing.");
}

void CLTracer::trace(float *resultImage)
{
	size_t globalWorkSize[2];
	globalWorkSize[0] = CLUtil::GetGlobalWorkSize(width, localWorkSize[0]);
	globalWorkSize[1] = CLUtil::GetGlobalWorkSize(height, localWorkSize[1]);

	cl_int counter = 0;
	cl_int clError;

	clError = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *) &image);
	V_RETURN_CL(clError, "Failed to set kernel arguments");

	clError = clEnqueueNDRangeKernel(commandQueue, kernel, 2, nullptr, globalWorkSize, localWorkSize, 0, nullptr, nullptr);
	V_RETURN_CL(clError, "Failed to enqueue kernel task");

	clError = clEnqueueReadBuffer(commandQueue, image, CL_TRUE, 0, width * height * 3 * sizeof(float), resultImage, 0, nullptr, nullptr);
	V_RETURN_CL(clError, "Failed to read image buffer back to CPU");

	clFinish(commandQueue);
}

void CLTracer::loadImage(float *imageData, int imageWidth, int imageHeight)
{
	cl_int clError;

	image = clCreateBuffer(context, CL_MEM_COPY_HOST_PTR | CL_MEM_READ_WRITE, imageWidth * imageHeight * 3 * sizeof(float), imageData, &clError);

//	clError = clEnqueueWriteBuffer(commandQueue, image, CL_TRUE, 0, imageWidth * imageHeight * sizeof(cl_float3), imageData, 0, nullptr, nullptr);
	V_RETURN_CL(clError, "Failed to write image to GPU");

	width = imageWidth;
	height = imageHeight;
}

