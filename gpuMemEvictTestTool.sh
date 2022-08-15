#!/usr/bin/env bash
CURRENT_DIR=`pwd`

IMAGE_NAME=suspend_resume_image
LP_CONTAINTER_NAME=suspend_resume_lp
HP_CONTAINTER_NAME=suspend_resume_hp

DOCKER_OPTS="--rm  -v ${CURRENT_DIR}/output:/workspace/output --privileged \
		-v /dev:/dev -v /sys/kernel/debug/:/sys/kernel/debug \
		-v ${CURRENT_DIR}/cl_cache:/workspace/cl_cache \
		-a stdout -a stderr"

LP_MEM_RATIO=$1
HP_MEM_RATIO=$2

export LD_LIBRARY_PATH=/usr/local/lib/:${LD_LIBRARY_PATH}

mkdir -p ${CURRENT_DIR}/output
rm -rf {CURRENT_DIR}/output/*

docker rm -f ${LP_CONTAINTER_NAME} ${HP_CONTAINTER_NAME} 2> /dev/null

pkill -9 -f sysMemMonitor
./sysMemMonitor &

printf  "\nStarting the low priority docker container\n"

docker run ${DOCKER_OPTS} -p 8080:8080 --name ${LP_CONTAINTER_NAME} ${IMAGE_NAME}  \
 	/usr/bin/bash -c "source ~/.bashrc && ./kernelCompiler && ./gpuMemEvictTestTool -m ${LP_MEM_RATIO} " &

while true
do 
	sleep 20

	printf "\nSuspending the low priority docker container\n"

	docker kill --signal STOP ${LP_CONTAINTER_NAME}

	printf "\nStarting the high priority docker container\n"
	
	docker run ${DOCKER_OPTS} -p 8081:8081 --name ${HP_CONTAINTER_NAME} ${IMAGE_NAME}  \
 		/usr/bin/bash -c "source ~/.bashrc && ./kernelCompiler && ./gpuMemEvictTestTool -m ${HP_MEM_RATIO} -t 15 -h"

	printf "\nResuming the low priority docker container\n"

	docker kill --signal CONT ${LP_CONTAINTER_NAME}
done
