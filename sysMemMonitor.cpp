#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <prometheus/counter.h>
#include <prometheus/exposer.h>
#include <prometheus/registry.h>

using namespace prometheus;
using namespace std::chrono;

#define KB 	1024u
#define MB 	( 1024u * KB)
#define GB 	( 1024u * MB)

int main() {
	struct sysinfo memInfo;
	
	Exposer exposer{"0.0.0.0:8082"};
  	auto registry = std::make_shared<Registry>();

	auto& gauge = BuildGauge()
							.Name("memory_eviction_monitoring")
							.Help("memory Eviction monitoring")
							.Register(*registry);

	auto& g = gauge.Add({{"mem_eviction_system_used_mem_gbytes", "System Used Memeory"}});

  	exposer.RegisterCollectable(registry);


	while (true) {	
		sysinfo(&memInfo);

		float memUsed = (memInfo.totalram - memInfo.freeram) * memInfo.mem_unit;

		memUsed /= GB;

		g.Set(memUsed);

		sleep(1);
	}

}