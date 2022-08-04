#----------------------------------------------------------------------------------------------------------------------
# Flags
#----------------------------------------------------------------------------------------------------------------------
SHELL:=/bin/bash

CURRENT_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
ONEAPI_ROOT ?= /opt/intel/oneapi

export TERM=xterm

CXX_COMPILER=dpcpp
CXXFLAGS=-g -Wno-c++20-extensions -Wno-deprecated-declarations -Wno-return-type
LDFLAGS=-lOpenCL -lpthread

DOCKER_IMAGE_NAME ?= suspend_resume_image

LP_MEM_RATIO ?= 0.5
HP_MEM_RATIO ?= 0.8

 
#----------------------------------------------------------------------------------------------------------------------
# Targets
#----------------------------------------------------------------------------------------------------------------------
default: run 
.PHONY: gpuMemEvictTestTool kernelCompiler

	
gpuMemEvictTestTool:
	@$(call msg,Building gpuMemEvictTestTool application  ...)
	@bash -c 'source ${ONEAPI_ROOT}/setvars.sh --force &> /dev/null && \
		$(CXX_COMPILER) $(CXXFLAGS) $@.cpp -o $@ $(LDFLAGS)' && \
	cp $@ /usr/bin

kernelCompiler:
	@$(call msg,Building the kernel compiler   ...)
	@bash -c 'source ${ONEAPI_ROOT}/setvars.sh --force &> /dev/null && \
		$(CXX_COMPILER) $(CXXFLAGS) $@.cpp -o $@ $(LDFLAGS)' && \
	cp $@ /usr/bin


build: kernelCompiler gpuMemEvictTestTool

run: 
	@$(call msg,Running the gpuMemEvictTestTool application ...)
	@./gpuMemEvictTestTool.sh ${LP_MEM_RATIO} ${HP_MEM_RATIO}

show:
	@python3  plot.py
	
clean:
	@rm -rf gpuMemEvictTestTool kernelCompiler

#----------------------------------------------------------------------------------------------------------------------
# Docker
#----------------------------------------------------------------------------------------------------------------------
docker-build:
	@$(call msg, Building the docker image  ${DOCKER_IMAGE_NAME} ...)
	@docker build  -t ${DOCKER_IMAGE_NAME} . --force-rm
	
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

