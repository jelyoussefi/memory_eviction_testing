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

int main(int argc, char* argv[]) {
	
	if ( argc != 2 ) {
		printf("Usage : %s -s | -r ", argv[0]);
		exit(0);
	}

	Exposer exposer{"0.0.0.0:8083"};

  	auto registry = std::make_shared<Registry>();

	auto& gauge = BuildGauge()
							.Name("memory_eviction_monitoring")
							.Help("memory Eviction monitoring")
							.Register(*registry);

	auto& suspend_gauche = gauge.Add({{"label", "suspend"}});
	auto& resume_gauche = gauge.Add({{"label", "resume)"}});

	if (!strcmp(argv[1], "-s")) {
		suspend_gauche.Set(16);
	}
	else if (!strcmp(argv[1], "-r")) {
		resume_gauche.Set(16);
	}
	else {
		printf("Usage : %s -s | -r ", argv[0]);
		exit(0);
	}
	
  	exposer.RegisterCollectable(registry);

	return 0;

}
