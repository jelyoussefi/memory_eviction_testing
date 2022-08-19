#define CL_TARGET_OPENCL_VERSION 220
#include <CL/cl_ext.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream> 
#include <experimental/filesystem>

#define MAX_SOURCE_SIZE 200000


static cl_int getFirstAvailableDevice(cl_device_type type_device, cl_device_id& device_id)
{
	int bFoundGPU = 0;
	int i, dc, dg;
	cl_device_id *accDevices;
	cl_int err;
	cl_uint numPlatforms;
	char dname[1024];
	cl_uint numDevices, entries;
	cl_ulong long_entries;
	size_t p_size;

	// Set up platform
	cl_platform_id allPlatformID[6], curPlatformID;

	numPlatforms = 0;
	err = clGetPlatformIDs(6, allPlatformID, &numPlatforms);
	if (err != CL_SUCCESS) {
		std::cout << "failed to query platforms. Error: " << std::to_string(err) << std::endl;
		exit(1);
	}

	/*obtain list of devices available on platform*/
	for (i = 0; i < numPlatforms; i++) {
		numDevices = 0;
		curPlatformID = allPlatformID[i];
		clGetDeviceIDs(curPlatformID, type_device, 0, NULL, &numDevices);
		accDevices = (cl_device_id *)malloc(sizeof(cl_device_id)*numDevices);

		err = clGetDeviceIDs(curPlatformID, CL_DEVICE_TYPE_GPU, numDevices, accDevices, NULL);
		if (err == CL_SUCCESS) {
			//for (int dg = 0; dg < numDevices; dg++)
			{
				device_id = accDevices[0];
				bFoundGPU = 1;
				break;
			}
		 	std::cout << "\t continue GPU... \n" <<std::endl; 
		}
	}
	if ( bFoundGPU == 0) {
		std::cout << "No GPU found. Exit\n";
		err = 1;
	}

	return err;
}


int main(int argc, char* argv[])
{
	cl_int err;

	if ( std::experimental::filesystem::exists("./cl_cache/matmul.bin") ) {
		return 0;
	}

	cl_device_id device_id;
	err = getFirstAvailableDevice(CL_DEVICE_TYPE_GPU, device_id);
	if (err != CL_SUCCESS) {
		std::cout << "No GPU found. Exit\n";
		return -1;
	}

	cl_context context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
	if (err != CL_SUCCESS) {
		std::cout << "failed to init context. Error: " << std::to_string(err) << std::endl;
		return -1;
	}

    	auto command_queue = clCreateCommandQueue(context, device_id, 0, NULL);


	FILE* file = fopen("matmul.cl", "rb");
	if (file == NULL) {
		std::cout << "failed to open program file" << std::endl;
		return -1;
	}

	const char* source = (char*)malloc(MAX_SOURCE_SIZE);
	size_t source_size = fread((void*)source, 1, MAX_SOURCE_SIZE, file);
	fclose(file);

	cl_program program = clCreateProgramWithSource(context, 1, &source, NULL, &err);
	if (err != CL_SUCCESS) {
		std::cout << "failed to Create Program - Error: " << std::to_string(err) << std::endl;
		return -1;
	}

	// compile kernel
	err = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
	if (err != CL_SUCCESS) {
		printf("Failed to compile OpenCL program %d\n", err);
		return -1;
	}

	cl_kernel kernel = clCreateKernel(program, "matrixMul", &err);
	if (err != CL_SUCCESS) {
		printf("failed to create kernel\n");
	}
	free((void*)source);

    size_t binary_size;
 	err = clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES, sizeof(size_t), &binary_size, NULL);
 	if (err != CL_SUCCESS) {
		printf("failed to get program binary sizes\n");
	}
    
    char *binary = (char*)malloc(binary_size);
    err = clGetProgramInfo(program, CL_PROGRAM_BINARIES, binary_size, &binary, NULL);
    if (err != CL_SUCCESS) {
		printf("failed to get program binary\n");
	}
    
    file = fopen("./cl_cache/matmul.bin", "wb");
    fwrite(binary, binary_size, 1, file);
    fclose(file);
    free(binary);

	return 0;
}
