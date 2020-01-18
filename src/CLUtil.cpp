/******************************************************************************
GPU Computing / GPGPU Praktikum source code.

******************************************************************************/

#include "CLUtil.h"

#include <iostream>
#include <fstream>
#include <cmath>
#include <OpenCL/cl.hpp>


///////////////////////////////////////////////////////////////////////////////
// CLUtil

cl::NDRange CLUtil::GetGlobalWorkSize(std::vector<size_t> dataElementCounts, cl::NDRange localWorkSize)
{
	std::vector<size_t> globalWorkSizes;

	for (int i = 0; i < dataElementCounts.size(); i++)
	{

		size_t r = dataElementCounts[i] % localWorkSize[i];
		if (r == 0)
			globalWorkSizes.push_back(dataElementCounts[i]);
		else
			globalWorkSizes.push_back(dataElementCounts[i] + localWorkSize[i] - r);
	}

	switch (globalWorkSizes.size())
	{
	case 1:
		return cl::NDRange(globalWorkSizes[0]);
	case 2:
		return cl::NDRange(globalWorkSizes[0], globalWorkSizes[1]);
	case 3:
		return cl::NDRange(globalWorkSizes[0], globalWorkSizes[1], globalWorkSizes[2]);
	default:
		return cl::NDRange();
	}
}

bool CLUtil::LoadProgramSourceToMemory(const std::string &Path, std::string &SourceCode)
{
	std::ifstream sourceFile;

	sourceFile.open(Path.c_str());
	if (!sourceFile.is_open())
	{
		std::cerr << "Failed to open file '" << Path << "'." << std::endl;
		return false;
	}

	// read the entire file into a string
	sourceFile.seekg(0, std::ios::end);
	std::ifstream::pos_type fileSize = sourceFile.tellg();
	sourceFile.seekg(0, std::ios::beg);

	SourceCode.clear();
	SourceCode.resize((size_t) fileSize);
	sourceFile.read(&SourceCode[0], fileSize);

	return true;
}

cl_program CLUtil::BuildCLProgramFromMemory(cl_device_id Device, cl_context Context, const std::string &SourceCode, const std::string &CompileOptions)
{

	// Ignore the last parameter CompileOptions in assignment 1
	// This may be used later to pass flags and macro definitions to the OpenCL compiler

	cl_program prog = nullptr;

	std::string srcSolution = SourceCode;

	const char *src = srcSolution.c_str();
	size_t length = srcSolution.size();

	cl_int clError;
	prog = clCreateProgramWithSource(Context, 1, &src, &length, &clError);
	if (CL_SUCCESS != clError)
	{
		std::cerr << "Failed to create CL program from source.";
		return nullptr;
	}

	// program created, now build it:
	const char *pCompileOptions = CompileOptions.size() > 0 ? CompileOptions.c_str() : nullptr;
	clError = clBuildProgram(prog, 1, &Device, pCompileOptions, nullptr, nullptr);
	if (CL_SUCCESS != clError)
	{
		std::cerr << "Failed to build CL program.";
		SAFE_RELEASE_PROGRAM(prog);
		return nullptr;
	}


	return prog;
}

#define CL_ERROR(x) case (x): return #x;

const char *CLUtil::GetCLErrorString(cl_int CLErrorCode)
{
	switch (CLErrorCode)
	{
	CL_ERROR(CL_SUCCESS);
	CL_ERROR(CL_DEVICE_NOT_FOUND);
	CL_ERROR(CL_DEVICE_NOT_AVAILABLE);
	CL_ERROR(CL_COMPILER_NOT_AVAILABLE);
	CL_ERROR(CL_MEM_OBJECT_ALLOCATION_FAILURE);
	CL_ERROR(CL_OUT_OF_RESOURCES);
	CL_ERROR(CL_OUT_OF_HOST_MEMORY);
	CL_ERROR(CL_PROFILING_INFO_NOT_AVAILABLE);
	CL_ERROR(CL_MEM_COPY_OVERLAP);
	CL_ERROR(CL_IMAGE_FORMAT_MISMATCH);
	CL_ERROR(CL_IMAGE_FORMAT_NOT_SUPPORTED);
	CL_ERROR(CL_BUILD_PROGRAM_FAILURE);
	CL_ERROR(CL_MAP_FAILURE);
	CL_ERROR(CL_INVALID_VALUE);
	CL_ERROR(CL_INVALID_DEVICE_TYPE);
	CL_ERROR(CL_INVALID_PLATFORM);
	CL_ERROR(CL_INVALID_DEVICE);
	CL_ERROR(CL_INVALID_CONTEXT);
	CL_ERROR(CL_INVALID_QUEUE_PROPERTIES);
	CL_ERROR(CL_INVALID_COMMAND_QUEUE);
	CL_ERROR(CL_INVALID_HOST_PTR);
	CL_ERROR(CL_INVALID_MEM_OBJECT);
	CL_ERROR(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR);
	CL_ERROR(CL_INVALID_IMAGE_SIZE);
	CL_ERROR(CL_INVALID_SAMPLER);
	CL_ERROR(CL_INVALID_BINARY);
	CL_ERROR(CL_INVALID_BUILD_OPTIONS);
	CL_ERROR(CL_INVALID_PROGRAM);
	CL_ERROR(CL_INVALID_PROGRAM_EXECUTABLE);
	CL_ERROR(CL_INVALID_KERNEL_NAME);
	CL_ERROR(CL_INVALID_KERNEL_DEFINITION);
	CL_ERROR(CL_INVALID_KERNEL);
	CL_ERROR(CL_INVALID_ARG_INDEX);
	CL_ERROR(CL_INVALID_ARG_VALUE);
	CL_ERROR(CL_INVALID_ARG_SIZE);
	CL_ERROR(CL_INVALID_KERNEL_ARGS);
	CL_ERROR(CL_INVALID_WORK_DIMENSION);
	CL_ERROR(CL_INVALID_WORK_GROUP_SIZE);
	CL_ERROR(CL_INVALID_WORK_ITEM_SIZE);
	CL_ERROR(CL_INVALID_GLOBAL_OFFSET);
	CL_ERROR(CL_INVALID_EVENT_WAIT_LIST);
	CL_ERROR(CL_INVALID_EVENT);
	CL_ERROR(CL_INVALID_OPERATION);
	CL_ERROR(CL_INVALID_GL_OBJECT);
	CL_ERROR(CL_INVALID_BUFFER_SIZE);
	CL_ERROR(CL_INVALID_MIP_LEVEL);
	default:
		return "Unknown error code";
	}
}

///////////////////////////////////////////////////////////////////////////////
