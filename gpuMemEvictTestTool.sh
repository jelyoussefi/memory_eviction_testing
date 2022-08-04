#!/usr/bin/env bash
CURRENT_DIR=`pwd`

IMAGE_NAME=suspend_resume_image
LP_CONTAINTER_NAME=suspend_resume_lp
HP_CONTAINTER_NAME=suspend_resume_hp

DOCKER_OPTS="--rm  -v ${CURRENT_DIR}/output:/workspace/output --privileged -v /dev:/dev -v /sys/kernel/debug/:/sys/kernel/debug -a stdout -a stderr"

LP_MEM_RATIO=$1
HP_MEM_RATIO=$2

mkdir -p ${CURRENT_DIR}/output
rm -rf {CURRENT_DIR}/output/*

printf  "\nStarting the low priority docker container\n"

docker run ${DOCKER_OPTS} --name ${LP_CONTAINTER_NAME} ${IMAGE_NAME}  \
 	/usr/bin/bash -c "source ~/.bashrc && kernelCompiler && gpuMemEvictTestTool -m ${LP_MEM_RATIO} -t 30" &

sleep 10

printf "\nSuspending the low priority docker container\n"

docker kill --signal STOP ${LP_CONTAINTER_NAME}

printf "\nStarting the high priority docker container\n"

docker run ${DOCKER_OPTS} --name ${HP_CONTAINTER_NAME} ${IMAGE_NAME}  \
 	/usr/bin/bash -c "source ~/.bashrc && kernelCompiler && gpuMemEvictTestTool -m ${HP_MEM_RATIO} -t 10 -h"
 	
printf "\nResuming the low priority docker container\n"

docker kill --signal CONT ${LP_CONTAINTER_NAME}

