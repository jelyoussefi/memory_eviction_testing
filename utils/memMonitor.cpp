#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream> 
#include <map>
#include <iterator>
#include <numeric>
#include <algorithm>
#include <math.h>
#include <ctime>
#include <regex>

#include <prometheus/counter.h>
#include <prometheus/exposer.h>
#include <prometheus/registry.h>

using namespace prometheus;
using namespace std::chrono;

#define KB 	1024u
#define MB 	( 1024u * KB)
#define GB 	( 1024u * MB)

float getGpuMemoryAllocatedSize() {
	double allocatedMemSize = 0;
	
	for (int t=0; t<=2; t++) {
		std::stringstream path;
		path << "/sys/kernel/debug/dri/" << t << "/i915_gem_objects";
		std::ifstream infile(path.str());
		for( std::string line; std::getline( infile, line ); ) {
			if ( line.find("local") != std::string::npos )  {
				std::regex seps("[ ,:]+");
	   			std::sregex_token_iterator rit(line.begin(), line.end(), seps, -1);
	    		auto tokens = std::vector<std::string>(rit, std::sregex_token_iterator());
	    		tokens.erase(std::remove_if(tokens.begin(), tokens.end(), [](std::string const& s){ return s.empty(); }), tokens.end());
	    		double totalMemSize, availableMemSize;
	    		std::istringstream(tokens[2]) >> std::hex >> totalMemSize; 
				std::istringstream(tokens[4]) >> std::hex >> availableMemSize;

				allocatedMemSize += (totalMemSize - availableMemSize);
		    }
		}
	}

	return allocatedMemSize/GB;
}

float getSystemMemoryAllocatedSize() {
	struct sysinfo memInfo;

	sysinfo(&memInfo);

	float memUsed = (memInfo.totalram - memInfo.freeram) * memInfo.mem_unit;

	return memUsed /= GB;
}


int main() {
	
	Exposer exposer{"0.0.0.0:8082"};
  	auto registry = std::make_shared<Registry>();

	auto& gauge = BuildGauge()
							.Name("memory_eviction_monitoring")
							.Help("memory Eviction monitoring")
							.Register(*registry);

	auto& sydMem_gauche = gauge.Add({{"label", "System memeory (GB)"}});
	auto& gpuMem_gauche = gauge.Add({{"label", "GPU memeory (GB)"}});

  	exposer.RegisterCollectable(registry);

	while (true) {	

		sydMem_gauche.Set( getSystemMemoryAllocatedSize() );

		gpuMem_gauche.Set( getGpuMemoryAllocatedSize() );

		sleep(1);
	}

}
