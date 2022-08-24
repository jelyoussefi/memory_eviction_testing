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


export LD_LIBRARY_PATH:=/usr/local/lib:${LD_LIBRARY_PATH}
#----------------------------------------------------------------------------------------------------------------------
# Targets
#----------------------------------------------------------------------------------------------------------------------
default: build 
.PHONY: prometheus-cpp gpuMemEvictTestTool kernelCompiler

prometheus-cpp:
	@$(call msg,Building  prometheus-cpp ...)
	@cd /tmp && rm -rf prometheus-cpp && git clone https://github.com/jupp0r/prometheus-cpp && \
		cd prometheus-cpp && git submodule init && git submodule update && \
		mkdir build && cd build && CXX=g++ \
		cmake .. -DBUILD_SHARED_LIBS=ON -DENABLE_PUSH=OFF -DENABLE_COMPRESSION=OFF .. && \
		make && sudo make install

gpuMemEvictTestTool:
	@$(call msg,Building gpuMemEvictTestTool application  ...)
	@bash -c 'source ${ONEAPI_ROOT}/setvars.sh --force &> /dev/null && \
		$(CXX_COMPILER) $(CXXFLAGS) $@.cpp -o $@ $(LDFLAGS)'

kernelCompiler: prometheus-cpp
	@$(call msg,Building the kernel compiler   ...)
	@bash -c 'source ${ONEAPI_ROOT}/setvars.sh --force &> /dev/null && \
		$(CXX_COMPILER) $(CXXFLAGS) $@.cpp -o $@ $(LDFLAGS)'


suspend_resume_prom: prometheus-cpp
	@$(call msg,Building the suspend/resume prometheus client  ...)
	@bash -c 'source ${ONEAPI_ROOT}/setvars.sh --force &> /dev/null && \
		$(CXX_COMPILER) $(CXXFLAGS) $@.cpp -o $@ $(LDFLAGS) && \
		cp $@ /usr/bin'



build: prometheus-cpp kernelCompiler gpuMemEvictTestTool suspend_resume_prom

clean:
	@rm -rf gpuMemEvictTestTool kernelCompiler suspend_resume_prom

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

