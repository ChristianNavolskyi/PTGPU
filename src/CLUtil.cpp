/******************************************************************************
GPU Computing / GPGPU Praktikum source code.

******************************************************************************/

#include "CLUtil.h"

#include <iostream>
#include <fstream>
#include <cmath>


///////////////////////////////////////////////////////////////////////////////
// CLUtil

size_t CLUtil::GetGlobalWorkSize(size_t DataElemCount, size_t LocalWorkSize)
{
	size_t r = DataElemCount % LocalWorkSize;
	if (r == 0)
		return DataElemCount;
	else
		return DataElemCount + LocalWorkSize - r;
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
	PrintBuildLog(prog, Device);
	if (CL_SUCCESS != clError)
	{
		std::cerr << "Failed to build CL program.";
		SAFE_RELEASE_PROGRAM(prog);
		return nullptr;
	}


	return prog;
}

void CLUtil::PrintBuildLog(cl_program Program, cl_device_id Device)
{
	cl_build_status buildStatus;
	clGetProgramBuildInfo(Program, Device, CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &buildStatus, nullptr);

	// let's print out possible warnings even if the kernel compiled..
	//if(buildStatus == CL_SUCCESS)
	//	return;

	//there were some errors.
	size_t logSize;
	clGetProgramBuildInfo(Program, Device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &logSize);
	std::string buildLog(logSize, ' ');

	clGetProgramBuildInfo(Program, Device, CL_PROGRAM_BUILD_LOG, logSize, &buildLog[0], nullptr);
	buildLog[logSize] = '\0';

	if (buildStatus != CL_SUCCESS)
		std::cout << "There were build errors!" << std::endl;
	std::cout << "Build log:" << std::endl;
	std::cout << buildLog << std::endl;
}


void CLUtil::LoadAndPrintFloat4Array(cl_command_queue CommandQueue, cl_mem array, unsigned int size)
{
	// avoid memory problems
	unsigned int actualPrintSize = std::min(size, (unsigned int) 256);
	// For debugging result
	auto *result = new cl_float4[actualPrintSize];
	// Read array from memory and print it out completely
	V_RETURN_CL(clEnqueueReadBuffer(CommandQueue, array, CL_TRUE, 0, sizeof(cl_float4) * actualPrintSize, result, 0, nullptr, nullptr), "Copying result back failed.");
	CLUtil::PrintArray(result, actualPrintSize);
}

void CLUtil::LoadAndPrintIntArray(cl_command_queue CommandQueue, cl_mem array, unsigned int size)
{
	// avoid memory problems
	unsigned int actualPrintSize = std::min(size, (unsigned int) 32);
	// For debugging result
	auto *result = new uint[actualPrintSize];
	// Read array from memory and print it out completely
	V_RETURN_CL(clEnqueueReadBuffer(CommandQueue, array, CL_TRUE, 0, sizeof(uint) * actualPrintSize, result, 0, nullptr, nullptr), "Copying result back failed.");
	CLUtil::PrintArray(result, actualPrintSize);
}

void CLUtil::PrintArray(cl_float4 *array, unsigned int size)
{
	std::cout << "Printing Array of size " << size << std::endl;

	for (unsigned int i = 0; i < size; i++)
	{
		std::cout << " array[" << i << "]:\ts[0]: " << array[i].s[0] << "\t\t\ts[1]: " << array[i].s[1] << "\t\t\ts[2]: " << array[i].s[2] << "\t\t\ts[3]: " << array[i].s[3] << std::endl;
	}

	std::cout << std::endl
			  << std::endl;
}

void CLUtil::PrintArray(uint *array, unsigned int size)
{
	std::cout << "Printing Array of size " << size << std::endl;

	for (unsigned int i = 0; i < size; i++)
	{
		std::cout << array[i] << " ";
	}

	std::cout << std::endl
			  << std::endl;
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
