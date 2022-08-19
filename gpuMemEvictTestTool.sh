#!/usr/bin/env bash
CURRENT_DIR=`pwd`

IMAGE_NAME=suspend_resume_image
LP_CONTAINTER_NAME=suspend_resume_lp
HP_CONTAINTER_NAME=suspend_resume_hp

DOCKER_OPTS="--rm  -v ${CURRENT_DIR}/output:/workspace/output --privileged \
		-v /dev:/dev -v /sys/kernel/debug/:/sys/kernel/debug \
		-v ${CURRENT_DIR}/cl_cache:/workspace/apps/test/cl_cache \
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

docker run ${DOCKER_OPTS} -p 8080:8080 --name ${LP_CONTAINTER_NAME} ${IMAGE_NAME} ./lp_entry_point.sh &

while true
do 
	sleep 20

	printf "\nSuspending the low priority docker container\n"

	docker kill --signal="SIGUSR1" ${LP_CONTAINTER_NAME}

	printf "\nStarting the high priority docker container\n"
	
	docker run ${DOCKER_OPTS} -p 8081:8081 --name ${HP_CONTAINTER_NAME} ${IMAGE_NAME} ./hp_entry_point.sh
	
	
	printf "\nResuming the low priority docker container\n"

	docker kill --signal="SIGUSR2" ${LP_CONTAINTER_NAME}
done
