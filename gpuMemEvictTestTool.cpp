#define CL_TARGET_OPENCL_VERSION 220
#include <CL/cl_ext.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>
#include <iomanip>
#include <csignal>
#include <vector>
#include <iostream>
#include <fstream>
#include <ctime>
#include <chrono>
#include <thread>


using namespace std::chrono;
using Clock = std::chrono::high_resolution_clock;


//----------------------------------------------------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------------------------------------------------

#define KB 	1024u
#define MB 	( 1024u * KB)
#define GB 	( 1024u * MB)

#define HTYPE float
#define MAX_SOURCE_SIZE 100000

#define MAX_VECTOR_SIZE  128*1024*1024L // 128MB memory size in byte

//#define N MAX_VECTOR_SIZE

#define dprintf(level, ...) \
	if ( level <= debug_level ) { \
		printf(__VA_ARGS__); \
	}

#define RED 	"\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define RESET   "\033[0m"

#define TIMER_START(suffix) 										\
	Clock::time_point t_start##suffix = Clock::now();

#define TIMER_STOP(suffix, name, ...) 								{ 		\
		timeDisplay(name, timeElapsed(t_start##suffix), ##__VA_ARGS__); 	\
		t_start##suffix = Clock::now();										\
	}

#define TIMER_RESET(suffix) \
	t_start##suffix = time_now();


#define PRIO_TO_NAME()   ( highPrio ? RED : GREEN ) << ( highPrio ? "High Priority " : "Low Priority ") << RESET

//----------------------------------------------------------------------------------------------------------------------
// Typedefs
//----------------------------------------------------------------------------------------------------------------------
typedef struct {
	cl_mem A;
	cl_mem B;
	cl_mem C;
} matrix_t;

//----------------------------------------------------------------------------------------------------------------------
// Local data
//----------------------------------------------------------------------------------------------------------------------
static bool highPrio = false;
static int debug_level = 1;
static std::ofstream file;

//----------------------------------------------------------------------------------------------------------------------
// Local functions
//----------------------------------------------------------------------------------------------------------------------
static float timeElapsed(Clock::time_point t_start) {
	Clock::time_point t_stop = Clock::now();
	std::chrono::duration<double, std::milli> diff = duration_cast<std::chrono::duration<double>>(t_stop - t_start);
	return diff.count();
}

static void timeDisplay(std::string name, float duration, uint32_t level=1) {
	if (level <= debug_level) {
		std::cout<<"\t"<<PRIO_TO_NAME()<< " "<< name<<" : \t"<<std::fixed<<std::setprecision(1)<<BLUE<<duration<<RESET<<" ms" << std::endl;	
	}
}


static void printBuildLog(cl_program vadd_kernel_program, cl_device_id device_id) {
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

static std::string getDeviceName(cl_device_id device) {
	char device_string[1024];

	// CL_DEVICE_NAME
	clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_string), &device_string, NULL);

	return device_string;
}

static cl_ulong getDeviceMemorySize(cl_device_id device) {
	
	cl_ulong mem_size;
	clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(mem_size), &mem_size, NULL);
	return mem_size;
}

static cl_mem createBuffer(cl_context ctx, cl_mem_flags flags, size_t size, void *host_ptr, pid_t slavePid=-1) {
	cl_int err;
	cl_mem buf = clCreateBuffer(ctx, flags, size, host_ptr, &err);

	if (err != CL_SUCCESS) {
		std::cout << "failed to create cl buffer of size" << size/MB <<  " MB, Error: " << std::to_string(err) << std::endl;
		return nullptr;
	}

 	return buf;
}


static void signalHandler( int signum ) {
   std::cout << "Interrupt signal (" << signum << ") received" << std::endl;

   // cleanup and close up stuff here  
   // terminate program  
}

static void record(uint32_t val) {
	auto ts = duration_cast< milliseconds >(system_clock::now().time_since_epoch()).count();
	file << std::fixed<<std::setprecision(3) << ts << "\t" << val << std::endl;
}

static void activity(bool* running) {
	record(0);

	while(*running) {
		record(1);
		usleep(500000);
	}

	record(0);

}


static void printDeviceInfo(cl_device_id device) {

	if ( debug_level <= 2 ) {
		return;
	}

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
	clGetDeviceInfo(device, CL_DEVICE_IMAGE2D_MAX_HEIGHT, sizeof(size_t), &szMaxDims[1], NULL);
	clGetDeviceInfo(device, CL_DEVICE_IMAGE3D_MAX_WIDTH, sizeof(size_t), &szMaxDims[2], NULL);
	clGetDeviceInfo(device, CL_DEVICE_IMAGE3D_MAX_HEIGHT, sizeof(size_t), &szMaxDims[3], NULL);
	clGetDeviceInfo(device, CL_DEVICE_IMAGE3D_MAX_DEPTH, sizeof(size_t), &szMaxDims[4], NULL);

	printf("\t  CL_DEVICE_PREFERRED_VECTOR_WIDTH_<t>\t");
	cl_uint vec_width[6];
	clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, sizeof(cl_uint), &vec_width[0], NULL);
	clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, sizeof(cl_uint), &vec_width[1], NULL);
	clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, sizeof(cl_uint), &vec_width[2], NULL);
	clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, sizeof(cl_uint), &vec_width[3], NULL);
	clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, sizeof(cl_uint), &vec_width[4], NULL);
	clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, sizeof(cl_uint), &vec_width[5], NULL);
	
}

void printf_exit(char* errmsg, cl_int err)
{
	printf(errmsg, err);
	exit(1);
}

cl_int getFirstAvailableDevice(cl_device_type type_device, cl_device_id& device_id)
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

	float memRatio = 0.5f;
	float duration = 60;
	pid_t slavePid = -1;

	int c;
    while ((c = getopt(argc, argv, "m:t:p:")) != -1) {

    	switch (c) {
            case 'm':
                memRatio = std::stof(optarg);
            	break;
            case 't':
                duration = std::stof(optarg);
            	break;
            case 'p':
                slavePid = atoi(optarg);
            	break;
            default:           
                std::cout << "Got unknown parse returns: " << c << std::endl; 
                exit(0);   
    	}
    }

	highPrio = (slavePid != -1);
	duration *= 1000; 
	cl_int err;

	std::string filename = (highPrio ? "./highPrio.dat" : "lowPrio.dat");
	file.open (filename);


	bool running = true;
	std::thread thr(activity, &running);
	signal(SIGSTOP, signalHandler);  
	signal(SIGCONT, signalHandler);  

	// Set up a GPU device if available, exit if not GPU
	cl_device_id device_id;
	err = getFirstAvailableDevice(CL_DEVICE_TYPE_GPU, device_id);
	if (err != CL_SUCCESS) {
		std::cout << "No GPU found. Exit\n";
		return -1;
	}

	const size_t buffSize = 512 * MB;
	cl_ulong deviceMemSize = getDeviceMemorySize(device_id);
	auto memSize = deviceMemSize * memRatio;
	size_t nbOperations =  (memSize)/(3*buffSize);

	std::cout << "\n----------------------------------------------------------------------------" << std::endl;
	std::cout << "\t"<< PRIO_TO_NAME() << "application "  << YELLOW << "started" << RESET << std::endl;
	std::cout << "\t\t  Device Name :\t" << getDeviceName(device_id) << std::endl;
	std::cout << "\t\t  Mem Size    :\t" << (float)deviceMemSize/GB << " GB"<<std::endl;
	std::cout << "\t\t  Pid         :\t" << getpid() << std::endl;
	std::cout << "\t\t  Required Mem:\t" << (float)(memSize)/GB << " GB" << std::endl;
	std::cout << "    ------------------------------------------------------------------------" << std::endl;
	if ( slavePid != -1 ) {
			std::cout << "\t" << PRIO_TO_NAME() << YELLOW << ": Suspending Process "  << RESET << slavePid << std::endl;
			kill(slavePid, SIGSTOP);
			std::cout << "    ------------------------------------------------------------------------" << std::endl;
		}

	printDeviceInfo(device_id);
    
    auto startTime = Clock::now();
    cl_context context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
	if (err != CL_SUCCESS) {
		std::cout << "failed to init context. Error: " << std::to_string(err) << std::endl;
		return -1;
	}

	cl_command_queue q;

	if (highPrio) {
		q = clCreateCommandQueue(context, device_id, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE, &err);
	}
	else {
	 	cl_command_queue_properties profilingQueueProperties[] = 
	 		{ CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, CL_QUEUE_PRIORITY_KHR, CL_QUEUE_PRIORITY_LOW_KHR, 0 };
	 	q = clCreateCommandQueueWithProperties(context, device_id, profilingQueueProperties, &err);
	 }

	if (err != CL_SUCCESS) {
		std::cout << "failed to init queue. Error: " << std::to_string(err) << std::endl;
		return -1;
	}

	/// main body

	float* inBuff = new float[buffSize/sizeof(float)]();
	for(size_t j = 0; j < buffSize/sizeof(float); j++) 	{
		inBuff[j] = 1.0f;
	}

	
    TIMER_START(creationTime);

	std::cout << "\t\t" << PRIO_TO_NAME() << ": Creating buffers "  << std::endl;

	std::vector<matrix_t> operations;

	for(int i = 0; i < nbOperations; i++) {
	
		matrix_t mat;
		mat.A  = createBuffer(context, CL_MEM_READ_ONLY  | CL_MEM_COPY_HOST_PTR, buffSize, inBuff); 
		mat.B  = createBuffer(context, CL_MEM_READ_ONLY  | CL_MEM_COPY_HOST_PTR, buffSize, inBuff); 
		mat.C  = createBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR, buffSize, NULL); 

		err = clFinish(q);
		if (err != CL_SUCCESS) {
			std::cout << "failed to finish unmap buff " << std::endl;
			return -1;
		}

		assert(mat.A != nullptr && mat.B != nullptr && mat.C != nullptr);
	
		operations.push_back(mat);

	}

    //TIMER_STOP(creationTime, "creation time" );
    
    delete[] inBuff;

	std::cout << "\t\t" << PRIO_TO_NAME() << ": Building the kernels "  << std::endl;

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
				printBuildLog(vadd_kernel_program, device_id);
			}
		}

		kernel = clCreateKernel(vadd_kernel_program, "vadd", &err);
		if (err != CL_SUCCESS) {
			printf("failed to create kernel\n");
		}
		free((void*)vadd_source);
	}


	if(err == CL_SUCCESS) {
		std::cout << "\t\t" << PRIO_TO_NAME() << ": Running the application "  << std::endl;

		size_t gws = buffSize/sizeof(float);
		float* outBuff = new float[buffSize/sizeof(float)]();
	
		uint32_t oLoop = 0;

		while( timeElapsed(startTime) < duration ) {
 
			for(size_t i = 0; i < operations.size(); i++) {
				cl_event evt;
				cl_ulong enqstart = 0;
				cl_ulong enqend = 0;

				memset(outBuff, 0, buffSize);
				auto mat = operations[i];

				err |= clSetKernelArg(kernel, 0, sizeof(mat.A), &mat.A ); 
				err |= clSetKernelArg(kernel, 1, sizeof(mat.B), &mat.B ); 
				err |= clSetKernelArg(kernel, 2, sizeof(mat.C), &mat.C );

				err |= clEnqueueNDRangeKernel(q, kernel, 1, nullptr, &gws, NULL, 0, nullptr, &evt);
				err |= clWaitForEvents(1, &evt);

				if (err != CL_SUCCESS) {
					std::cout << "Exec Error: " << std::to_string(err) << std::endl;
				}

				err |= clGetEventProfilingInfo(evt, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &enqstart, NULL);
				err |= clGetEventProfilingInfo(evt, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &enqend, NULL);
		
				if (err != CL_SUCCESS) {
					std::cout << "Exec Error: " << std::to_string(err) << std::endl;
				}

				float cmdQEnQTime = float((cl_double)(enqend - enqstart) * (cl_double)(1e-09));
				if ( debug_level >= 2 ) {
					std::cout << "\t\t" << PRIO_TO_NAME() << std::to_string(i) << " time " << std::to_string(cmdQEnQTime) << std::endl ;
				}
				clReleaseEvent(evt);

				err = clEnqueueReadBuffer(q, mat.C, CL_TRUE, 0, buffSize, outBuff, 0, NULL, NULL);

				for(size_t j = 0; j < buffSize/sizeof(float); j++) 	{
					if ( outBuff[j] != 2.0f ) {
						std::cout << RED << "\tMatrix addition failed" << RESET << std::endl;
						break;
					}
					
				}
			}

			if ( debug_level >= 1 ) {
				std::cout << "\t\t\t" << PRIO_TO_NAME() << "Loop : " << std::to_string(oLoop) << std::endl ;
			}
			oLoop++;
		}

		delete[] outBuff;
	}


	//cleanup
	for(size_t i = 0; i < operations.size(); i++) {
		clReleaseMemObject(operations[i].A);
		clReleaseMemObject(operations[i].B);
		clReleaseMemObject(operations[i].C);
	}

	clReleaseCommandQueue(q);
	clReleaseContext(context);

	std::cout << "\n----------------------------------------------------------------------------" << std::endl;
	std::cout << "\t"<< PRIO_TO_NAME() << "application "  << YELLOW << "ended" << RESET << std::endl;
	std::cout << "----------------------------------------------------------------------------" << std::endl;

	if ( slavePid != -1 ) {
		std::cout << "\t" << PRIO_TO_NAME() << YELLOW << ": Resuming Process "  << RESET << slavePid << std::endl;
		kill(slavePid, SIGCONT);
		std::cout << "    ------------------------------------------------------------------------" << std::endl;
	}

	running = false;
	thr.join();
	file.close();

	return 0;
}
