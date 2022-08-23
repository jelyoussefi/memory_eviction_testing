#!/usr/bin/env bash
CURRENT_DIR=`pwd`

IMAGE_NAME=suspend_resume_image
LP_CONTAINTER_NAME=suspend_resume_lp
HP_CONTAINTER_NAME=suspend_resume_hp

LP_MEM_RATIO=$1
HP_MEM_RATIO=$2
HP_DURATION=$3

DOCKER_OPTS="--rm  -v ${CURRENT_DIR}/output:/workspace/output --privileged \
		-v /dev:/dev -v /sys/kernel/debug/:/sys/kernel/debug \
		-v ${CURRENT_DIR}/cl_cache:/workspace/apps/test/cl_cache \
		-a stdout -a stderr \
		--env LP_MEM_RATIO=${LP_MEM_RATIO} --env HP_MEM_RATIO=${HP_MEM_RATIO} --env HP_DURATION=${HP_DURATION} "


export LD_LIBRARY_PATH=/usr/local/lib/:${LD_LIBRARY_PATH}

docker rm -f ${LP_CONTAINTER_NAME} ${HP_CONTAINTER_NAME} 2> /dev/null


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
