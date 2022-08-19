#----------------------------------------------------------------------------------------------------------------------
# Flags
#----------------------------------------------------------------------------------------------------------------------
SHELL:=/bin/bash

CURRENT_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
ONEAPI_ROOT ?= /opt/intel/oneapi

export TERM=xterm

CXX_COMPILER=g++
CXXFLAGS=-g -Wno-c++20-extensions -Wno-deprecated-declarations -Wno-return-type 
LDFLAGS=-lOpenCL -lpthread -lstdc++fs -lprometheus-cpp-core -lprometheus-cpp-pull


DOCKER_BASE_IMAGE ?= ge/intel/sles_dpcpp_compiler_host_level_zero
DOCKER_IMAGE_NAME ?= suspend_resume_image


LP_MEM_RATIO ?= 0.8
HP_MEM_RATIO ?= 0.8

export LD_LIBRARY_PATH:=/usr/local/lib:${LD_LIBRARY_PATH}
#----------------------------------------------------------------------------------------------------------------------
# Targets
#----------------------------------------------------------------------------------------------------------------------
default: run 
.PHONY:  sysMemMonitor grafana


sysMemMonitor:
	@$(call msg,Building the system memory monitor compiler   ...)
	@bash -c 'source ${ONEAPI_ROOT}/setvars.sh --force &> /dev/null && \
		$(CXX_COMPILER) $(CXXFLAGS) $@.cpp -o $@ $(LDFLAGS)'

grafana:
	@$(call msg,Starting grapfana  ...)
	@mkdir -p ${CURRENT_DIR}/grafana/data
	@chmod 777 ${CURRENT_DIR}/grafana/data
	@cd ${CURRENT_DIR}/grafana && \
	 	docker-compose down && \
		docker-compose up -d
	

build:
	@make -C ./apps/test build
	
run: sysMemMonitor grafana
	@$(call msg,Running the gpuMemEvictTestTool application ...)
	@sudo ./gpuMemEvictTestTool.sh ${LP_MEM_RATIO} ${HP_MEM_RATIO}

	
clean:
	@make -C ./apps/test clean

#----------------------------------------------------------------------------------------------------------------------
# Docker
#----------------------------------------------------------------------------------------------------------------------
docker-build:
	@$(call msg, Building the docker image  ${DOCKER_IMAGE_NAME} ...)
	@docker build  --build-arg BASE_IMAGE=${DOCKER_BASE_IMAGE}  -t ${DOCKER_IMAGE_NAME} . --force-rm
	
#----------------------------------------------------------------------------------------------------------------------
# helper functions
#----------------------------------------------------------------------------------------------------------------------
define msg
	tput setaf 2 && \
	for i in $(shell seq 1 120 ); do echo -n "-"; done; echo  "" && \
	echo "         "$1 && \
	for i in $(shell seq 1 120 ); do echo -n "-"; done; echo "" && \
	tput sgr0
endef

