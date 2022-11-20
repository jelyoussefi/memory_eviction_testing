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
#include <algorithm>
#include <numeric>

//----------------------------------------------------------------------------------------------------------------------------

#define MB (1024*1024)
#define TDIFF(tb, ta) (tb.tv_sec - ta.tv_sec + 0.000001*(tb.tv_usec - ta.tv_usec))
using namespace sycl;

#define MB (1024*1024)
#define GB (1024*1024*1024)

#define MBDIFF 25000000
#define RED   	"\x1B[1;31m"
#define GREEN   "\x1B[1;32m"
#define YELLOW  "\x1B[1;33m"
#define BLUE   	"\x1B[1;34m"
#define RESET   "\x1B[0m"
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

static const char* color(int id) {
	static const char* colors[] = {GREEN, RED, BLUE, YELLOW};
	if ( id < 0 || id >= sizeof(colors)/sizeof(char*) ) {
		id = 0;
	}
	return colors[id];
}

//----------------------------------------------------------------------------------------------------------------------------
static bool
memInit( queue &q, std::vector <uint32_t*>& vBuffers, std::vector <uint32_t>& rangeBuffers, 
		 std::vector<uint32_t>& indexes, uint32_t value, uint32_t delay=0 ) {

	bool status = true;
	for(int i = 0; i < vBuffers.size(); i++) {

		int idx = i;
		if ( indexes.size() > 0 ) {
			if ( i < indexes.size() ) {
				idx = indexes[i];
			}
			else {
				break;
			}
		}

		q.submit([&](auto &h) {
		    	
			auto acc_mem = vBuffers[idx];

			h.parallel_for(range(rangeBuffers[idx]), [=](auto index) {
				acc_mem[index] = value;
			});

		}).wait();
		
		if ( delay > 0 ) {
			usleep(delay*1000);
		}

	}

	return status;
}

static bool
memRead( queue &q, std::vector <uint32_t*>& vBuffers, std::vector <uint32_t>& rangeBuffers, 
	     std::vector<uint32_t>& indexes, uint32_t value, uint32_t delay=0 ) {

    	bool status = true;
    	{
    		buffer<bool, 1> buf_status(&status, 1);
			for(int i = 0; i < vBuffers.size() ; i++) {
				int idx = i;
				if ( indexes.size() > 0 ) {
					if ( i < indexes.size() ) {
						idx = indexes[i];
					}
					else {
						break;
					}
				}
				q.submit([&](auto &h) {

					auto acc_status = buf_status.get_access<access::mode::write>(h);
					auto acc_mem = vBuffers[idx];

					h.parallel_for(range(rangeBuffers[idx]), [=](auto index) {
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

static void
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

static bool 
process(queue& q, uint64_t globalSize, uint64_t allocSize, bool random, uint32_t duration, uint32_t id=0, bool verbose = true) {
    
    std::vector <uint32_t*> bufVec;
    std::vector <uint32_t> rangeBuffers;
 	uint32_t delay = 0;
    uint32_t i = 0;
    std::vector<float> times;

    std::cout << "\tCreating Buffers ..." << std::endl;
    
    createBuffs(q, globalSize, allocSize, bufVec, rangeBuffers);
   
    std::cout << "\tStarting loops" << std::endl;

    auto startTime = Time::now();


    while( duration==0 || (timeElapsed(startTime)/1000 < duration ) ) {

    	auto startLoopTime = Time::now();

    	std::vector<uint32_t> indexes((3*bufVec.size())/4);

    	if ( random ) {
    		std::srand(unsigned(std::time(nullptr)));
			std::generate(indexes.begin(), indexes.end(), [&]() { return rand() % indexes.size(); });

		}	
		
    	if ( i%16==0 ) {
    		float average = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
    		std::cout << color(id) << "\t\t Loop:\t"<< i << std::fixed<<std::setprecision(2)<<" ( "<< average << " s )"<<RESET << std::endl;
    	}
    	
		bool res = memInit(q, bufVec, rangeBuffers, indexes, i, delay);

		if (!res) {
			std::cout << RED << "\tMemory Init failed" << RESET << std::endl;
	     		return false;
		}

		res = memRead(q, bufVec, rangeBuffers, indexes, i, delay);
		if (!res) {
			std::cout << RED << "\tMemory Read failed" << RESET << std::endl;
	     		return false;
		}
		times.push_back(timeElapsed(startLoopTime));
		i++;
    }
  
    q.wait();
    
    for (int i=0; i<bufVec.size(); i++ ) {
	    sycl::free(bufVec[i], q);
    }
   
   	printf("\tend\n");
    return true;

}

static void
printUsage(char *prog) {
	printf("%s [-m mem_ration] [-b block-size] [-r]\n", prog);
}

int main(int argc, char* argv[]) {
	bool random = false;
	float memRatio = 1.0;
	uint64_t memBlokSize = 2048;
	uint32_t duration = 0;
	uint32_t id = 0;

	int c;
	while ((c = getopt (argc, argv, "m:b:t:i:r")) != -1)
    switch (c)
	{
	case 'm':
			memRatio = std::stof(optarg);
			break;
		case 'b':
			memBlokSize = std::stoi(optarg);
			break;
		case 'r':
			random = true;
			break;
		case 't':
			duration = std::stoi(optarg);
			break;
		case 'i':
			id = std::stoi(optarg);
			break;
		case 'h':
			printUsage(argv[0]);
		return 0;

		default:
		printUsage(argv[0]);
		return -1;
	}


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

    uint64_t memGlobalSize = d.get_info<sycl::info::device::global_mem_size>();
    uint64_t memRequiredSize = memGlobalSize*memRatio;

    std::cout << "\n---------------------------------------------------------------------------------" << std::endl;
    std::cout <<"\t"<<"Device :\t"<< color(id) <<d.get_info<sycl::info::device::name>() << RESET << std::endl;
    std::cout <<"\t"<<"\tGlobal mem size     :\t"   << std::fixed<<std::setprecision(2) <<  color(id) << (float)memGlobalSize/GB << " Gb"<< RESET << std::endl;
    std::cout <<"\t"<<"\tRequired mem size   :\t"<< std::fixed<<std::setprecision(2) << color(id) << (float)memRequiredSize/GB <<" Bb" << RESET << std::endl;
    std::cout <<"\t"<<"\tAlloc block size    :\t"   << std::fixed<<std::setprecision(2) <<  color(id) << (float)memBlokSize << " Mb"<< RESET << std::endl;
    std::cout <<"\t"<<"\tProcess Id          :\t"   <<  color(id) << getpid() << RESET << std::endl;
    std::cout << "---------------------------------------------------------------------------------" << std::endl;
    process(q, memRequiredSize, memBlokSize*MB, random, duration, id, true);


    return 0;
}
