#----------------------------------------------------------------------------------------------------------------------
# Flags
#----------------------------------------------------------------------------------------------------------------------
SHELL:=/bin/bash

CURRENT_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
ONEAPI_ROOT ?= /opt/intel/oneapi

export TERM=xterm

CXX_COMPILER=dpcpp
CXXFLAGS=-g -Wno-c++20-extensions -Wno-deprecated-declarations
LDFLAGS=-lOpenCL -lpthread

LP_MEM_RATIO ?= 0.5
HP_MEM_RATIO ?= 0.8

#----------------------------------------------------------------------------------------------------------------------
# Targets
#----------------------------------------------------------------------------------------------------------------------
default: run 
.PHONY: gpuMemEvictTestTool

	
gpuMemEvictTestTool:
	@$(call msg,Building gpuMemEvictTestTool application  ...)
	@bash -c 'source ${ONEAPI_ROOT}/setvars.sh --force &> /dev/null && \
		$(CXX_COMPILER) $(CXXFLAGS) $@.cpp -o $@ $(LDFLAGS)'

build: gpuMemEvictTestTool

run: gpuMemEvictTestTool
	@$(call msg,Running the gpuMemEvictTestTool application ...)
	@mkdir -p ./output/
	@sudo bash -c 'source ${ONEAPI_ROOT}/setvars.sh --force &> /dev/null && \
		ZE_AFFINITY_MASK=1.0 ./gpuMemEvictTestTool.sh ${LP_MEM_RATIO} ${HP_MEM_RATIO}'

show:
	@python3  plot.py
clean:
	@rm -rf gpuMemEvictTestTool 

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

