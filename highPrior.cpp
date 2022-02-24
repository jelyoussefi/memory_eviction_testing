#define CL_TARGET_OPENCL_VERSION 220
#include <CL/cl.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <ctime>
#include <chrono>

using Clock = std::chrono::high_resolution_clock;


#define HTYPE float
#define MAX_SOURCE_SIZE 100000
#define MAX_SYNC_KERNELS 20
#define KERNEL_NUM 2

#define MAX_VECTOR_SIZE  128*1024*1024L // 128MB memory size in byte
#define SKIP_ITEMS_NUM 256*1024 // 256K

#define N MAX_VECTOR_SIZE
size_t skip_items = SKIP_ITEMS_NUM;
int nKernels = KERNEL_NUM;

void PrintBuildLog(cl_program vadd_kernel_program, cl_device_id device_id) {
	// Determine the size of the log
	size_t log_size;
	clGetProgramBuildInfo(vadd_kernel_program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

	// Allocate memory for the log
	char *log = (char *)malloc(log_size);

	// Get the log
	clGetProgramBuildInfo(vadd_kernel_program, device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

	// Print the log
	printf("%s\n", log);

	free(log);
}

void PrintKernelInfo(cl_kernel kernel, cl_device_id device_id)
{
	size_t kernel_size = 0;
	//clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(kernel_size), &kernel_size, NULL);
	//printf("\t  CL_KERNEL_WORK_GROUP_SIZE:\t\t%lu items\n", (unsigned int)(kernel_size));

	//clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_GLOBAL_WORK_SIZE, sizeof(kernel_size), &kernel_size, NULL);
	//printf("\t  CL_KERNEL_GLOBAL_WORK_SIZE:\t\t%lu items\n", (unsigned int)(kernel_size));
}

void PrintDeviceInfo(cl_device_id device) {
	char device_string[1024];

	std::cout << "\tPrinting devices Information\n";

	// CL_DEVICE_NAME
	clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_string), &device_string, NULL);
	std::cout << "\t  CL_DEVICE_NAME: \t\t\t" << device_string << std::endl;

	// CL_DEVICE_VENDOR
	clGetDeviceInfo(device, CL_DEVICE_VENDOR, sizeof(device_string), &device_string, NULL);
	std::cout << "\t  CL_DEVICE_NAME: \t\t\t" << device_string << std::endl;

	// CL_DRIVER_VERSION
	clGetDeviceInfo(device, CL_DRIVER_VERSION, sizeof(device_string), &device_string, NULL);
	std::cout << "\t  CL_DEVICE_NAME: \t\t\t" << device_string << std::endl;

	// CL_DEVICE_INFO
	cl_device_type type;
	clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(type), &type, NULL);
	if (type & CL_DEVICE_TYPE_CPU)
		printf("\t  CL_DEVICE_TYPE:\t\t\t%s\n", "CL_DEVICE_TYPE_CPU");
	if (type & CL_DEVICE_TYPE_GPU)
		printf("\t  CL_DEVICE_TYPE:\t\t\t%s\n", "CL_DEVICE_TYPE_GPU");
	if (type & CL_DEVICE_TYPE_ACCELERATOR)
		printf("\t  CL_DEVICE_TYPE:\t\t\t%s\n", "CL_DEVICE_TYPE_ACCELERATOR");
	if (type & CL_DEVICE_TYPE_DEFAULT)
		printf("\t  CL_DEVICE_TYPE:\t\t\t%s\n", "CL_DEVICE_TYPE_DEFAULT");

	// CL_DEVICE_MAX_COMPUTE_UNITS
	cl_uint compute_units;
	clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(compute_units), &compute_units, NULL);
	printf("\t  CL_DEVICE_MAX_COMPUTE_UNITS:\t\t%u\n", compute_units);

	// CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS
	size_t workitem_dims;
	clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(workitem_dims), &workitem_dims, NULL);
	//printf("\t  CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS:\t%u\n", workitem_dims);

	// CL_DEVICE_MAX_WORK_ITEM_SIZES
	size_t workitem_size[3];
	clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(workitem_size), &workitem_size, NULL);
	//printf("\t  CL_DEVICE_MAX_WORK_ITEM_SIZES:\t%u / %u / %u \n", workitem_size[0], workitem_size[1], workitem_size[2]);

	// CL_DEVICE_MAX_WORK_GROUP_SIZE
	size_t workgroup_size;
	clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(workgroup_size), &workgroup_size, NULL);
	//printf("\t  CL_DEVICE_MAX_WORK_GROUP_SIZE:\t%u\n", workgroup_size);

	// CL_DEVICE_MAX_CLOCK_FREQUENCY
	cl_uint clock_frequency;
	clGetDeviceInfo(device, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(clock_frequency), &clock_frequency, NULL);
	printf("\t  CL_DEVICE_MAX_CLOCK_FREQUENCY:\t%u MHz\n", clock_frequency);

	// CL_DEVICE_ADDRESS_BITS
	cl_uint addr_bits;
	clGetDeviceInfo(device, CL_DEVICE_ADDRESS_BITS, sizeof(addr_bits), &addr_bits, NULL);
	printf("\t  CL_DEVICE_ADDRESS_BITS:\t\t%u\n", addr_bits);

	// CL_DEVICE_MAX_MEM_ALLOC_SIZE
	cl_ulong max_mem_alloc_size;
	clGetDeviceInfo(device, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(max_mem_alloc_size), &max_mem_alloc_size, NULL);
	printf("\t  CL_DEVICE_MAX_MEM_ALLOC_SIZE:\t\t%u MByte\n", (unsigned int)(max_mem_alloc_size / (1024 * 1024)));

	// CL_DEVICE_GLOBAL_MEM_SIZE
	cl_ulong mem_size;
	clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(mem_size), &mem_size, NULL);
	printf("\t  CL_DEVICE_GLOBAL_MEM_SIZE:\t\t%u MByte\n", (unsigned int)(mem_size / (1024 * 1024)));

	// CL_DEVICE_ERROR_CORRECTION_SUPPORT
	cl_bool error_correction_support;
	clGetDeviceInfo(device, CL_DEVICE_ERROR_CORRECTION_SUPPORT, sizeof(error_correction_support), &error_correction_support, NULL);
	printf("\t  CL_DEVICE_ERROR_CORRECTION_SUPPORT:\t%s\n", error_correction_support == CL_TRUE ? "yes" : "no");

	// CL_DEVICE_LOCAL_MEM_TYPE
	cl_device_local_mem_type local_mem_type;
	clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_TYPE, sizeof(local_mem_type), &local_mem_type, NULL);
	printf("\t  CL_DEVICE_LOCAL_MEM_TYPE:\t\t%s\n", local_mem_type == 1 ? "local" : "global");

	// CL_DEVICE_LOCAL_MEM_SIZE
	clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(mem_size), &mem_size, NULL);
	printf("\t  CL_DEVICE_LOCAL_MEM_SIZE:\t\t%u KByte\n", (unsigned int)(mem_size / 1024));

	// CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE
	clGetDeviceInfo(device, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof(mem_size), &mem_size, NULL);
	printf("\t  CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE:\t%u KByte\n", (unsigned int)(mem_size / 1024));

	// CL_DEVICE_QUEUE_PROPERTIES
	cl_command_queue_properties queue_properties;
	clGetDeviceInfo(device, CL_DEVICE_QUEUE_PROPERTIES, sizeof(queue_properties), &queue_properties, NULL);
	if (queue_properties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE)
		printf("\t  CL_DEVICE_QUEUE_PROPERTIES:\t\t%s\n", "CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE");
	if (queue_properties & CL_QUEUE_PROFILING_ENABLE)
		printf("\t  CL_DEVICE_QUEUE_PROPERTIES:\t\t%s\n", "CL_QUEUE_PROFILING_ENABLE");

	// CL_DEVICE_IMAGE_SUPPORT
	cl_bool image_support;
	clGetDeviceInfo(device, CL_DEVICE_IMAGE_SUPPORT, sizeof(image_support), &image_support, NULL);
	printf("\t  CL_DEVICE_IMAGE_SUPPORT:\t\t%u\n", image_support);

	// CL_DEVICE_MAX_READ_IMAGE_ARGS
	cl_uint max_read_image_args;
	clGetDeviceInfo(device, CL_DEVICE_MAX_READ_IMAGE_ARGS, sizeof(max_read_image_args), &max_read_image_args, NULL);
	printf("\t  CL_DEVICE_MAX_READ_IMAGE_ARGS:\t%u\n", max_read_image_args);

	// CL_DEVICE_MAX_WRITE_IMAGE_ARGS
	cl_uint max_write_image_args;
	clGetDeviceInfo(device, CL_DEVICE_MAX_WRITE_IMAGE_ARGS, sizeof(max_write_image_args), &max_write_image_args, NULL);
	printf("\t  CL_DEVICE_MAX_WRITE_IMAGE_ARGS:\t%u\n", max_write_image_args);

	// CL_DEVICE_IMAGE2D_MAX_WIDTH, CL_DEVICE_IMAGE2D_MAX_HEIGHT, CL_DEVICE_IMAGE3D_MAX_WIDTH, CL_DEVICE_IMAGE3D_MAX_HEIGHT, CL_DEVICE_IMAGE3D_MAX_DEPTH
	size_t szMaxDims[5];
	printf("\n\t  CL_DEVICE_IMAGE <dim>");
	clGetDeviceInfo(device, CL_DEVICE_IMAGE2D_MAX_WIDTH, sizeof(size_t), &szMaxDims[0], NULL);
	//printf("\t\t\t2D_MAX_WIDTH\t %u\n", szMaxDims[0]);
	clGetDeviceInfo(device, CL_DEVICE_IMAGE2D_MAX_HEIGHT, sizeof(size_t), &szMaxDims[1], NULL);
	//printf("\t\t\t\t\t\t2D_MAX_HEIGHT\t %u\n", szMaxDims[1]);
	clGetDeviceInfo(device, CL_DEVICE_IMAGE3D_MAX_WIDTH, sizeof(size_t), &szMaxDims[2], NULL);
	//printf("\t\t\t\t\t\t3D_MAX_WIDTH\t %u\n", szMaxDims[2]);
	clGetDeviceInfo(device, CL_DEVICE_IMAGE3D_MAX_HEIGHT, sizeof(size_t), &szMaxDims[3], NULL);
	//printf("\t\t\t\t\t\t3D_MAX_HEIGHT\t %u\n", szMaxDims[3]);
	clGetDeviceInfo(device, CL_DEVICE_IMAGE3D_MAX_DEPTH, sizeof(size_t), &szMaxDims[4], NULL);
	//printf("\t\t\t\t\t\t3D_MAX_DEPTH\t %u\n", szMaxDims[4]);

	// CL_DEVICE_PREFERRED_VECTOR_WIDTH_<type>
	printf("\t  CL_DEVICE_PREFERRED_VECTOR_WIDTH_<t>\t");
	cl_uint vec_width[6];
	clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, sizeof(cl_uint), &vec_width[0], NULL);
	clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, sizeof(cl_uint), &vec_width[1], NULL);
	clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, sizeof(cl_uint), &vec_width[2], NULL);
	clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, sizeof(cl_uint), &vec_width[3], NULL);
	clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, sizeof(cl_uint), &vec_width[4], NULL);
	clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, sizeof(cl_uint), &vec_width[5], NULL);
	//printf("CHAR %u, SHORT %u, INT %u, FLOAT %u, DOUBLE %u\n\n\n",
	//	vec_width[0], vec_width[1], vec_width[2], vec_width[3], vec_width[4]);
}

void printf_exit(char* errmsg, cl_int err)
{
	printf(errmsg, err);
	exit(1);
}

cl_int GetFirstAvailableDevice(cl_device_type type_device, cl_device_id& device_id)
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
	else {
		std::cout << "Platform OK " << numPlatforms <<  "\n";
	}

	/*obtain list of devices available on platform*/
	for (i = 0; i < numPlatforms; i++) {
		numDevices = 0;
		curPlatformID = allPlatformID[i];
		clGetDeviceIDs(curPlatformID, type_device, 0, NULL, &numDevices);
		std::cout << "\t" << std::to_string(numDevices) << " GPU devices found on platform  " << numDevices  << std::endl;
		accDevices = (cl_device_id *)malloc(sizeof(cl_device_id)*numDevices);

		//err = clGetDeviceIDs(curPlatformID, type_device, numDevices, &accDevices, NULL);
		//err = clGetDeviceIDs(curPlatformID, type_device, numDevices, accDevices, NULL);
		//err = clGetDeviceIDs(firstPlatformID, CL_DEVICE_TYPE_GPU, 1, &accDevices[dg], NULL);
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
		else {
			printf("\t\tError:Failure in clGetDeviceIds, error code = %d\n\t\t, Or,This platform cannot interact with the GPUs.Check for the drivers\n", err);
		}
	}
	if (bFoundGPU == 0) {
		std::cout << "No GPU found. Exit\n";
		err = 1;
	}

	return err;
}

int main(int argc, char* argv[])
{
	// get input parameter for number of kernels to run async
	if (argc > 1) {
		nKernels = atoi(argv[1]);
		if (nKernels > MAX_SYNC_KERNELS)
			nKernels = MAX_SYNC_KERNELS;
	}

	cl_int err;
	skip_items = SKIP_ITEMS_NUM;

	// Set up a GPU device if available, exit if not GPU
	cl_device_id device_id;
	err = GetFirstAvailableDevice(CL_DEVICE_TYPE_GPU, device_id);
	if (err != CL_SUCCESS) {
		std::cout << "No GPU found. Exit\n";
		return -1;
	}

	//PrintDeviceInfo(device_id);

	cl_context context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
	if (err != CL_SUCCESS) {
		std::cout << "failed to init context. Error: " << std::to_string(err) << std::endl;
		return -1;
	}

	//cl_command_queue_properties profilingQueueProperties =  CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE | CL_QUEUE_PRIORITY_LOW_KHR;
	//cl_command_queue_properties profilingQueueProperties =  (CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE);
	cl_command_queue q = clCreateCommandQueue(context, device_id, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE, &err);
	//cl_command_queue q = clCreateCommandQueueWithProperties(context, device_id, &profilingQueueProperties, &err);
	if (err != CL_SUCCESS) {
		std::cout << "failed to init queue. Error: " << std::to_string(err) << std::endl;
		return -1;
	}

/// main body
	constexpr size_t kiloByte = 1024u;
	constexpr size_t megaByte = 1024u * kiloByte;
	constexpr size_t gigaByte = 1024u * megaByte;

	const size_t buffSize = 512 * megaByte;
	const size_t maxSources = 24 ;//6*16;
        size_t sourcesCreated = 0;
	const cl_mem_flags memFlags = CL_MEM_READ_WRITE;

	std::cout << "\nHIGH PRIOR init - buffs: " << std::to_string(maxSources) << std::endl;

	cl_mem sources[maxSources] = {0};
	float* inBuff = new float[buffSize/sizeof(float)]();
	for(size_t j = 0; j < buffSize/sizeof(float); j++) 	{
		inBuff[j] = 1.0f;
	}

        Clock::time_point startTime = Clock::now();


	for(size_t i = 0; i < maxSources ; i++)
	{
		//sources[i] = clCreateBuffer(context, memFlags, buffSize, nullptr, &err);
		sources[i] = clCreateBuffer(context, memFlags | CL_MEM_COPY_HOST_PTR, buffSize, inBuff, &err);
		if (err != CL_SUCCESS) {
			std::cout << "failed to create buff: " << std::to_string(i) <<  " Error: " << std::to_string(err) << std::endl;
			break;
		}
#if 0
		float *pBufferBase = (float *)clEnqueueMapBuffer(q, sources[i], CL_TRUE, CL_MAP_WRITE, 0, buffSize, 0, NULL, NULL, &err);
		if (err != CL_SUCCESS) {
			std::cout << "failed to map buff: " << std::to_string(i) <<  " Error: " << std::to_string(err) << std::endl;
			break;
		}

		//std::copy(inBuff, inBuff + buffSize/sizeof(float), pBufferBase);

		err = clEnqueueUnmapMemObject(q, sources[i], pBufferBase, 0, NULL, NULL);
		if (err != CL_SUCCESS) {
			std::cout << "failed to unmap buff: " << std::to_string(i) <<  " Error: " << std::to_string(err) << std::endl;
			break;
		}
#endif// // no buff Map/Unmap
		err = clFinish(q);
		if (err != CL_SUCCESS) {
			std::cout << "failed to finish unmap buff: " << std::to_string(i) <<  " Error: " << std::to_string(err) << std::endl;
			break;
		}
  		sourcesCreated+=1;

	}

        Clock::time_point endTime =  Clock::now();

        std::chrono::duration<double> diffTime = std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime);
        double microsecondTime = diffTime.count() * 1000.0;
        std::cout << "Create Time HIGH:  " << std::to_string(microsecondTime) << std::endl ;

	delete[] inBuff;

	cl_kernel kernel;
	{
		// acquire kernel source
		cl_program vadd_kernel_program;
		FILE* vadd_source_file = fopen("vadd.cl", "rb");
		if (vadd_source_file == NULL) {
			std::cout << "failed to open program file" << std::endl;
		}
		const char* vadd_source = (char*)malloc(MAX_SOURCE_SIZE);
		size_t source_size = fread((void*)vadd_source, 1, MAX_SOURCE_SIZE, vadd_source_file);

		vadd_kernel_program = clCreateProgramWithSource(context, 1, &vadd_source, NULL, &err);
		if (err != CL_SUCCESS)
		{
			std::cout << "failed to Create Program - Error: " << std::to_string(err) << std::endl;
		}

		// compile kernel
		const char opts[1000] = {}; // custom compilation flags can go here
		err |= clBuildProgram(vadd_kernel_program, 1, &device_id, opts, NULL, NULL);
		if (err != CL_SUCCESS) {
			printf("Failed to compile OpenCL program %d\n", err);
			if (err == CL_BUILD_PROGRAM_FAILURE)
			{
				PrintBuildLog(vadd_kernel_program, device_id);
			}

		}

		kernel = clCreateKernel(vadd_kernel_program, "vadd", &err);
		if (err != CL_SUCCESS) {
			printf("failed to create kernel\n");
		}
		free((void*)vadd_source);
	}

	size_t gws = buffSize/sizeof(float);

	if(err == CL_SUCCESS)
	{
		std::cout << "### HIGH PRIOR EXEC ###\n";
		size_t oLoop = 0;
		const size_t maxLoops = 48*1;

		while( oLoop++ < maxLoops ) {

                        for(size_t iter = 0; iter < sourcesCreated;) {
                                cl_event evt;
                                cl_ulong enqstart = 0;
                                cl_ulong enqend = 0;

                                err |= clSetKernelArg(kernel, 0, sizeof(sources[iter + 0]), &sources[iter + 0]);
                                err |= clSetKernelArg(kernel, 1, sizeof(sources[iter + 1]), &sources[iter + 1] );
                                err |= clSetKernelArg(kernel, 2, sizeof(sources[iter + 2]), &sources[iter + 2] );
                                //err |= clEnqueueNDRangeKernel(q, kernel, 1, nullptr, &gws, NULL, 0, nullptr, nullptr);
                                //err |= clFinish(q);
                                err |= clEnqueueNDRangeKernel(q, kernel, 1, nullptr, &gws, NULL, 0, nullptr, &evt);
                                err |= clWaitForEvents(1, &evt);

                                if (err != CL_SUCCESS)
                                {
                                        std::cout << "Exec Error: " << std::to_string(err) << std::endl;
                                }
                                err |= clGetEventProfilingInfo(evt, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &enqstart, NULL);
                                err |= clGetEventProfilingInfo(evt, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &enqend, NULL);
                                if (err != CL_SUCCESS)
                                {
                                        std::cout << "Exec Error: " << std::to_string(err) << std::endl;
                                }
				float cmdQEnQTime = float((cl_double)(enqend - enqstart) * (cl_double)(1e-09));
				std::cout << "### HIGH PRIOR EXEC ###  " << std::to_string(iter/3) << " time " << std::to_string(cmdQEnQTime) << std::endl ;
				clReleaseEvent(evt);

				//iterator
				iter +=3;
			}
			std::cout << "HIGH PRIOR n " << std::to_string(oLoop) << std::endl ;
		}
	}

	std::cout << " ### HIGH PRIOR END Loop\n";

        for(size_t i = 0; i < sourcesCreated ; i++)
        {
                if(sources[i] != 0)
                {
                        clReleaseMemObject(sources[i]);
                }
                else { std::cout << "\t " << std::to_string(i) <<" Empty\n" ;}
        }

	clReleaseCommandQueue(q);
	clReleaseContext(context);

	std::cout << "\n###############\n HIGH PRIOR QUIT \n###############\n";

	return 0;
}
