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

LP_MEM_RATIO ?= 0.8
HP_MEM_RATIO ?= 0.8

 
#----------------------------------------------------------------------------------------------------------------------
# Targets
#----------------------------------------------------------------------------------------------------------------------
default: run 
.PHONY: gpuMemEvictTestTool kernelCompiler oneAPIMemTest openclMemTest

	
gpuMemEvictTestTool:
	@$(call msg,Building gpuMemEvictTestTool application  ...)
	@bash -c 'source ${ONEAPI_ROOT}/setvars.sh --force &> /dev/null && \
		$(CXX_COMPILER) $(CXXFLAGS) $@.cpp -o $@ $(LDFLAGS)'

kernelCompiler:
	@$(call msg,Building the kernel compiler   ...)
	@bash -c 'source ${ONEAPI_ROOT}/setvars.sh --force &> /dev/null && \
		$(CXX_COMPILER) $(CXXFLAGS) $@.cpp -o $@ $(LDFLAGS)'

oneAPIMemTest: 
	@$(call msg,Building oneAPIMemTest   ...)
	@bash -c 'source ${ONEAPI_ROOT}/setvars.sh --force &> /dev/null && \
		$(CXX_COMPILER) $(CXXFLAGS) -fsycl -O3  $@.cpp -o $@ '

openclMemTest: 
	@$(call msg,Building openclMemTest   ...)
	@bash -c 'source ${ONEAPI_ROOT}/setvars.sh --force &> /dev/null && \
		g++ $(CXXFLAGS)  -O3  $@.cpp -o $@ $(LDFLAGS)'
		
build: kernelCompiler gpuMemEvictTestTool

run: 
	@$(call msg,Running the gpuMemEvictTestTool application ...)
	@mkdir -p ./output/
	@sudo bash -c 'source ${ONEAPI_ROOT}/setvars.sh --force &> /dev/null && \
		./gpuMemEvictTestTool.sh ${LP_MEM_RATIO} ${HP_MEM_RATIO}'

show:
	@python3  plot.py
clean:
	@rm -rf gpuMemEvictTestTool kernelCompiler

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

