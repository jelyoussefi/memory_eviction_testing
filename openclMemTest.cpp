#define CL_TARGET_OPENCL_VERSION 220
#include <CL/cl_ext.h>

#include <unistd.h>
#include <chrono>
#include <cmath>
#include <regex>
#include <array>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sys/time.h>

#define BLOCKSIZE ((uint32_t)(1024*1024))
#define TYPE uint32_t
#define TDIFF(tb, ta) (tb.tv_sec - ta.tv_sec + 0.000001*(tb.tv_usec - ta.tv_usec))

#define GB (1024*1024*1024)
#define MB (1024*1024)
#define MBDIFF 25000000
#define RED   "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define RESET   "\033[0m"
#define CUDA_ENABLED std::getenv("CUDA_ENABLED") ? (strcmp("ON", std::getenv("CUDA_ENABLED"))==0) : false

#define MAX_SOURCE_SIZE 200000

using namespace std::chrono;

using Time = std::chrono::high_resolution_clock;

#define TIMER_START(suffix) \
    auto t_start##suffix = Time::now();

#define TIMER_STOP(suffix, name, verbose)                                                           \
    {                                                                                               \
        auto t_stop = Time::now();                                                                  \
        auto diff = duration_cast<milliseconds>(t_stop - t_start##suffix);                          \
        if (verbose)                                                                                \
            std::cout << "\t\t" << name << GREEN <<  diff.count() << " ms"<< RESET << std::endl;    \
        t_start##suffix = Time::now();                                                              \
    }

static float timeElapsed(Time::time_point t_start) {
	Time::time_point t_stop = Time::now();
	std::chrono::duration<double, std::milli> diff = duration_cast<std::chrono::duration<double>>(t_stop - t_start);
	return diff.count();
}

//----------------------------------------------------------------------------------------------------------
static const char* initMemKernel_source =
R""(
__kernel void d_memInit(__global unsigned long* dest, unsigned long val) {
    int i = get_global_id(0);
    dest[i] = val;
}
)"";

static const char* readMemKernel_source =
R""(
__kernel void d_memRead(__global unsigned long* src, unsigned long val) {
    int i = get_global_id(0);
    if ( src[i] != val ) {
    	return;
    }
}
)"";


static 
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

static 
cl_ulong getDeviceMemorySize(cl_device_id device) {
	
	cl_ulong mem_size;
	clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(mem_size), &mem_size, NULL);
	return mem_size;
}
bool
memInit(cl_command_queue q, cl_kernel kernel,
		std::vector <cl_mem>& vBuffers, 
        std::vector <size_t>& rangeBuffers, uint32_t value, uint32_t delay=0 ) {

	bool status = true;

	for(int i = 0; i < vBuffers.size(); i++){

		cl_int err;
		cl_event evt;
		err = clSetKernelArg(kernel, 0, sizeof(vBuffers[i]), &vBuffers[i] ); 
		err |= clSetKernelArg(kernel, 1, sizeof(value), &value ); 
		err |= clEnqueueNDRangeKernel(q, kernel, 1, nullptr, &rangeBuffers[i], NULL, 0, nullptr, &evt);
		err |= clWaitForEvents(1, &evt);
		if (err != CL_SUCCESS) {
			std::cout << "Exec Error: " << std::to_string(err) << std::endl;
			return -1;
		}

		clReleaseEvent(evt);

	}


	return status;
}

bool
memRead(cl_command_queue q, cl_kernel kernel, 
		std::vector <cl_mem>& vBuffers, 
        std::vector <size_t>& rangeBuffers, uint32_t value, uint32_t delay=0 ) {
	
	bool status = true;
	
	for(int i = 0; i < vBuffers.size() ; i++) {

		cl_int err;
		cl_event evt;
		err = clSetKernelArg(kernel, 0, sizeof(vBuffers[i]), &vBuffers[i] ); 
		err |= clSetKernelArg(kernel, 1, sizeof(value), &value ); 
		err |= clEnqueueNDRangeKernel(q, kernel, 1, nullptr, &rangeBuffers[i], NULL, 0, nullptr, &evt);
		err |= clWaitForEvents(1, &evt);
		if (err != CL_SUCCESS) {
			std::cout << "Exec Error: " << std::to_string(err) << std::endl;
			return -1;
		}

		clReleaseEvent(evt);
	}
	
    return status;
}

bool
createBuffs(cl_context ctx, uint64_t globalSize, uint64_t allocSize,  
	std::vector <cl_mem>& vBuffers, std::vector <size_t>& rangeBuffers) {

	cl_int err;

	for(uint32_t i = 0; i < globalSize/allocSize; i++) {

		cl_mem buf = clCreateBuffer(ctx, CL_MEM_READ_WRITE, allocSize, NULL, &err);

		if (err != CL_SUCCESS) {
			std::cout << "failed to create cl buffer of size" << allocSize/MB <<  " MB, Error: " << std::to_string(err) << std::endl;
			return false;
		}

		vBuffers.push_back(buf);
		rangeBuffers.push_back(allocSize/sizeof(uint32_t));
	}
   
	auto memLeft = globalSize - (vBuffers.size())*allocSize;
	if(memLeft>0) {
		cl_mem buf = clCreateBuffer(ctx, CL_MEM_READ_WRITE, memLeft, NULL, &err);

		if (err != CL_SUCCESS) {
			std::cout << "failed to create cl buffer of size" << memLeft/MB <<  " MB, Error: " << std::to_string(err) << std::endl;
			return false;
		}

		vBuffers.push_back(buf);
		rangeBuffers.push_back(allocSize/sizeof(uint32_t));
	}

	return true;
}

bool process(cl_context context, cl_command_queue q, cl_kernel initMem_kernel, cl_kernel readMem_kernel,
			 uint64_t globalSize, uint64_t allocSize, bool verbose = true) {
    
    std::vector <cl_mem> bufVec;
    std::vector <size_t> rangeBuffers;

    std::cout << "\tCreating Buffers ..." << std::endl;
    if (!createBuffs(context, globalSize, allocSize, bufVec, rangeBuffers) ) {
		std::cout << "failed to create buffers " << std::endl;
		exit(-1);
    }
    cl_int err = clFinish(q);
	if (err != CL_SUCCESS) {
		std::cout << "failed to finish unmap buff " << std::endl;
		return false;
	}
    std::cout << "\tStarting the application ..." << std::endl;

    uint32_t delay = 0;
    uint32_t i = 0;
    auto startTime = Time::now();
    while( true ) {
    	if (i%8 == 0) {
    		std::cout << "\t\t" << GREEN << ": Loop "  << i << RESET << std::endl;
    	}
    	
		bool res = memInit(q, initMem_kernel, bufVec, rangeBuffers, i, delay);

		if (!res) {
			std::cout << RED << "\tMemory Init failed" << RESET << std::endl;
	     		return false;
		}

		res = memRead(q, readMem_kernel, bufVec, rangeBuffers, i, delay);
		if (!res) {
			std::cout << RED << "\tMemory Read failed" << RESET << std::endl;
	     		return false;
		}
		i++;
    }
  
    
    std::cout << "Freeing memory" << std::endl;
    for (int i=0; i<bufVec.size(); i++ ) {
	   // sycl::free(bufVec[i], q);
    }
   
    return true;

}

int main() {
      
    cl_device_id device_id;
	cl_int err = getFirstAvailableDevice(CL_DEVICE_TYPE_GPU, device_id);
	if (err != CL_SUCCESS) {
		std::cout << "No GPU found. Exit\n";
		return -1;
	}

	cl_context context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
	if (err != CL_SUCCESS) {
		std::cout << "failed to init context. Error: " << std::to_string(err) << std::endl;
		return -1;
	}

	cl_command_queue q = clCreateCommandQueue(context, device_id,  CL_QUEUE_PROFILING_ENABLE, &err);

	auto mGlobalSize  = getDeviceMemorySize(device_id);
    uint64_t mAllocSize = 2L*GB;

    if(CUDA_ENABLED==0){
      mGlobalSize-=MBDIFF*sizeof(uint32_t);
    }
    mGlobalSize-=MBDIFF*sizeof(uint32_t);

    std::cout << "\n---------------------------------------------------------------------------------" << std::endl;
	std::cout << "\t"<< GREEN << "OpenCL memory testting" << RESET << " application " << YELLOW << "started" << RESET << std::endl;
    std::cout <<"\t\t  Global mem size :\t"   << std::fixed<<std::setprecision(2) << (float)mGlobalSize/GB << " Gb" << std::endl;
    std::cout <<"\t\t  Max allocation size :\t"<< std::fixed<<std::setprecision(2) << (float)mAllocSize/GB <<" Gb" << std::endl;
    std::cout <<"\t\t  Pid :\t"<< getpid() << std::endl;
    std::cout << "---------------------------------------------------------------------------------" << std::endl;
  
  	cl_program initMem_program = clCreateProgramWithSource(context, 1, &initMemKernel_source, NULL, &err);
	if (err != CL_SUCCESS) {
		std::cout << "failed to Create initMem Program - Error: " << std::to_string(err) << std::endl;
		return -1;
	}

	// compile kernel
	err = clBuildProgram(initMem_program, 1, &device_id, NULL, NULL, NULL);
	if (err != CL_SUCCESS) {
		//std::cout << "Failed to compile initMem OpenCL program - Error: " << std::to_string(err) << std::endl;
		//return -1;
	}
	
	cl_kernel initMem_kernel = clCreateKernel(initMem_program, "d_memInit", &err);
	if (err != CL_SUCCESS) {
		std::cout << "Failed to create initMem kernel - Error: " << std::to_string(err) << std::endl;
		return -1;
	}


	cl_program readMem_program = clCreateProgramWithSource(context, 1, &readMemKernel_source, NULL, &err);
	if (err != CL_SUCCESS) {
		std::cout << "failed to Create readMem Program - Error: " << std::to_string(err) << std::endl;
		return -1;
	}

	// compile kernel
	err = clBuildProgram(readMem_program, 1, &device_id, NULL, NULL, NULL);
	if (err != CL_SUCCESS) {
		std::cout << "Failed to compile readMem OpenCL program - Error: " << std::to_string(err) << std::endl;
		return -1;
	}


	cl_kernel readMem_kernel = clCreateKernel(readMem_program, "d_memRead", &err);
	if (err != CL_SUCCESS) {
		std::cout << "Failed to create readMem kernel - Error: " << std::to_string(err) << std::endl;
		return -1;
	}

    process(context, q, initMem_kernel, readMem_kernel, mGlobalSize, mAllocSize, true);

    return 0;
}
