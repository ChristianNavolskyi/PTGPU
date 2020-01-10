/******************************************************************************
                         .88888.   888888ba  dP     dP
                        d8'   `88  88    `8b 88     88
                        88        a88aaaa8P' 88     88
                        88   YP88  88        88     88
                        Y8.   .88  88        Y8.   .8P
                         `88888'   dP        `Y88888P'


   a88888b.                                         dP   oo
  d8'   `88                                         88
  88        .d8888b. 88d8b.d8b. 88d888b. dP    dP d8888P dP 88d888b. .d8888b.
  88        88'  `88 88'`88'`88 88'  `88 88    88   88   88 88'  `88 88'  `88
  Y8.   .88 88.  .88 88  88  88 88.  .88 88.  .88   88   88 88    88 88.  .88
   Y88888P' `88888P' dP  dP  dP 88Y888P' `88888P'   dP   dP dP    dP `8888P88
                                88                                        .88
                                dP                                    d8888P
******************************************************************************/

#pragma once

#include <OpenCL/opencl.h>

#include <string>
#include <iostream>
#include <algorithm>

//! Utility class for frequently-needed OpenCL tasks
class CLUtil {
public:
	//! Determines the OpenCL global work size given the number of data elements and threads per workgroup
	static size_t GetGlobalWorkSize(size_t DataElemCount, size_t LocalWorkSize);

	//! Loads a program source to memory as a string
	static bool LoadProgramSourceToMemory(const std::string &Path, std::string &SourceCode);

	//! Builds a CL program
	static cl_program BuildCLProgramFromMemory(cl_device_id Device, cl_context Context, const std::string &sourceCode, const std::string &compileOptions = "");

	static const char *GetCLErrorString(cl_int CLErrorCode);
};

// Some useful shortcuts for handling pointers and validating function calls
#define V_RETURN_FALSE_CL(expr, errmsg) do {cl_int e=(expr);if(CL_SUCCESS!=e){std::cerr<<"Error: "<<errmsg<<" ["<<CLUtil::GetCLErrorString(e)<<"]"<<std::endl; return false; }} while(0)
#define V_RETURN_CL_ERROR(expr, errmsg) do {cl_int e=(expr);if(CL_SUCCESS!=e){std::cerr<<"Error: "<<errmsg<<" ["<<CLUtil::GetCLErrorString(e)<<"]"<<std::endl; return e; }} while(0)
#define V_RETURN_0_CL(expr, errmsg) do {cl_int e=(expr);if(CL_SUCCESS!=e){std::cerr<<"Error: "<<errmsg<<" ["<<CLUtil::GetCLErrorString(e)<<"]"<<std::endl; return 0; }} while(0)
#define V_RETURN_MINUS_1_CL(expr, errmsg) do {cl_int e=(expr);if(CL_SUCCESS!=e){std::cerr<<"Error: "<<errmsg<<" ["<<CLUtil::GetCLErrorString(e)<<"]"<<std::endl; return -1; }} while(0)
#define V_RETURN_NULL_CL(expr, errmsg) do {cl_int e=(expr);if(CL_SUCCESS!=e){std::cerr<<"Error: "<<errmsg<<" ["<<CLUtil::GetCLErrorString(e)<<"]"<<std::endl; return NULL; }} while(0)
#define V_RETURN_CL(expr, errmsg) do {cl_int e=(expr);if(CL_SUCCESS!=e){std::cerr<<"Error: "<<errmsg<<" ["<<CLUtil::GetCLErrorString(e)<<"]"<<std::endl; return; }} while(0)

#define SAFE_DELETE(ptr) do {if(ptr){ delete ptr; ptr = NULL; }} while(0)
#define SAFE_DELETE_ARRAY(x) do {if(x){delete [] x; x = NULL;}} while(0)

#define SAFE_RELEASE_KERNEL(ptr)    do {if(ptr){ clReleaseKernel(ptr); ptr = NULL; }} while(0)
#define SAFE_RELEASE_PROGRAM(ptr)   do {if(ptr){ clReleaseProgram(ptr); ptr = NULL; }} while(0)
#define SAFE_RELEASE_MEMOBJECT(ptr) do {if(ptr){ clReleaseMemObject(ptr); ptr = NULL; }} while(0)
#define SAFE_RELEASE_SAMPLER(ptr)   do {if(ptr){ clReleaseSampler(ptr); ptr = NULL; }} while(0)
#define SAFE_RELEASE_CONTEXT(ptr)   do {if(ptr){ clReleaseContext(ptr); ptr = NULL; }} while(0)
#define SAFE_RELEASE_COMMAND_QUEUE(ptr)   do {if(ptr){ clReleaseCommandQueue(ptr); ptr = NULL; }} while(0)

#define ARRAYLEN(a) (sizeof(a)/sizeof(a[0]))
