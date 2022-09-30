#include <unistd.h>
#include <chrono>
#include <cmath>
#include <regex>
#include <array>
#include <iostream>
#include <iomanip>
#include "CL/sycl.hpp"
#include <ctime>
#include <sys/time.h>

#define BLOCKSIZE ((uint32_t)(1024*1024))
#define MB (1024*1024)
#define TYPE uint32_t
#define TDIFF(tb, ta) (tb.tv_sec - ta.tv_sec + 0.000001*(tb.tv_usec - ta.tv_usec))
using namespace sycl;

#define GB (1024*1024*1024)
#define MBDIFF 25000000
#define RED   "\033[1;31m"
#define GREEN   "\033[1;32m"
#define RESET   "\033[0m"
#define CUDA_ENABLED std::getenv("CUDA_ENABLED") ? (strcmp("ON", std::getenv("CUDA_ENABLED"))==0) : false


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

bool
memInit(queue &q, std::vector <uint32_t*>& vBuffers, 
        std::vector <uint32_t>& rangeBuffers, uint32_t value, uint32_t delay=0 ) {
	bool status = true;

	for(int i = 0; i < vBuffers.size(); i++){

		q.submit([&](auto &h) {
		    	
			auto acc_mem = vBuffers[i];

			h.parallel_for(range(rangeBuffers[i]), [=](auto index) {
				acc_mem[index] = value;
			});

		}).wait();
		
		if ( delay > 0 ) {
			usleep(delay*1000);
		}

	}

	return status;
}

bool
memRead(queue &q, std::vector <uint32_t*>& vBuffers, 
        std::vector <uint32_t>& rangeBuffers, uint32_t value, uint32_t delay=0 ) {
    	bool status = true;
    	{
    		buffer<bool, 1> buf_status(&status, 1);
		for(int i = 0; i < vBuffers.size() ; i++) {
			q.submit([&](auto &h) {

				auto acc_status = buf_status.get_access<access::mode::write>(h);
				auto acc_mem = vBuffers[i];

				h.parallel_for(range(rangeBuffers[i]), [=](auto index) {
				    if ( acc_mem[index] != value ) {
					acc_status[0] = false;
					return;
				    }
				});
			}).wait();
		    
			if ( delay > 0 ) {
				usleep(delay*1000);
			}
		}
	}
    

   	 return status;
}

void
createBuffs(queue &q, uint64_t globalSize, uint64_t allocSize,  
	std::vector <uint32_t*>& vBuffers, std::vector <uint32_t>& rangeBuffers) {
	    
	for(uint32_t i = 0; i < globalSize/allocSize; i++) {

		auto mem = sycl::malloc_device<uint32_t>(allocSize/sizeof(uint32_t),q);
		vBuffers.push_back(mem);
		rangeBuffers.push_back(allocSize/sizeof(uint32_t));
	}
   
	auto memLeft = globalSize - (vBuffers.size())*allocSize;
	if(memLeft>0) {
		vBuffers.push_back(sycl::malloc_device<uint32_t>(memLeft/sizeof(uint32_t),q));
		rangeBuffers.push_back(memLeft/sizeof(uint32_t));
	}
}

bool process(device& d, queue& q, uint64_t globalSize, uint64_t allocSize, bool verbose = true) {
    
    std::vector <uint32_t*> bufVec;
    std::vector <uint32_t> rangeBuffers;

    std::cout << "Creating Buffers ..." << std::endl;
    createBuffs(q, globalSize, allocSize, bufVec, rangeBuffers);
   
    std::cout << "Starting loops" << std::endl;
    uint32_t delay = 0;
    uint32_t i = 0;
    auto startTime = Time::now();
    while( true ) {
    	if (i%16==0) {
    		std::cout <<"Loop: "<< i << std::endl;
    	}
    	
	bool res = memInit(q, bufVec, rangeBuffers, i, delay);

	if (!res) {
		std::cout << RED << "\tMemory Init failed" << RESET << std::endl;
     		return false;
	}

	res = memRead(q, bufVec, rangeBuffers, i, delay);
	if (!res) {
		std::cout << RED << "\tMemory Read failed" << RESET << std::endl;
     		return false;
	}
	i++;
    }
  
    q.wait();
    
    std::cout << "Freeing memory" << std::endl;
    for (int i=0; i<bufVec.size(); i++ ) {
	    sycl::free(bufVec[i], q);
    }
   
    return true;

}

int main() {
      device d;
      try {
            d = device(gpu_selector());
      } catch (exception const& e) {
            std::cout << "Cannot select a GPU\n" << e.what() << "\n";
            std::cout << "Attempting to use CPU device\n";
            d = device(cpu_selector());
      }
  
    property_list q_prop{property::queue::in_order()};
    queue q(d, q_prop);

    uint64_t mAllocSize = d.get_info<sycl::info::device::max_mem_alloc_size>();
    uint64_t mGlobalSize = d.get_info<sycl::info::device::global_mem_size>();
    if(CUDA_ENABLED==0){
      mGlobalSize-=MBDIFF*sizeof(uint32_t);
    }
      mGlobalSize-=MBDIFF*sizeof(uint32_t);
    std::cout << "\n---------------------------------------------------------------------------------" << std::endl;
    std::cout <<"\t"<<"Device :\t"<< RED <<d.get_info<sycl::info::device::name>() << RESET << std::endl;
    std::cout <<"\t"<<"\tGlobal mem size :\t"   << std::fixed<<std::setprecision(2) <<  GREEN << (float)mGlobalSize/GB << " Gb"<< RESET << std::endl;
    std::cout <<"\t"<<"\tMax allocation size :\t"<< std::fixed<<std::setprecision(2) << GREEN << (float)mAllocSize/GB <<" Gb" << RESET << std::endl;
    std::cout <<"\t"<<"\tMax work group size :\t"<< GREEN << d.get_info<sycl::info::device::max_work_group_size>() << RESET << std::endl;
    std::cout << "---------------------------------------------------------------------------------" << std::endl;
    std::cout << "\tProcessing Times :" << std::endl;
    std::cout << "\t\tWarming up ..." << std::endl;

    process(d, q, mGlobalSize, mAllocSize, true);
    process(d, q, mGlobalSize, mAllocSize);

    return 0;
}
