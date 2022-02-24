#----------------------------------------------------------------------------------------------------------------------
# Flags
#----------------------------------------------------------------------------------------------------------------------
SHELL:=/bin/bash

CURRENT_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
ONEAPI_ROOT ?= /opt/intel/oneapi

export TERM=xterm

CXX_COMPILER=dpcpp
CXXFLAGS=
LDFLAGS=-lOpenCL
#----------------------------------------------------------------------------------------------------------------------
# Targets
#----------------------------------------------------------------------------------------------------------------------
default: run 
.PHONY: highPrior lowPrior


highPrior:
	@$(call msg,Building highPrior  ...)
	@bash -c 'source ${ONEAPI_ROOT}/setvars.sh --force 2> /dev/null && \
		$(CXX_COMPILER) $(CXXFLAGS) $@.cpp -o $@ $(LDFLAGS)'

lowPrior:
	@$(call msg,Building lowPrior  ...)
	@bash -c 'source ${ONEAPI_ROOT}/setvars.sh --force 2> /dev/null && \
		$(CXX_COMPILER) $(CXXFLAGS) $@.cpp -o $@ $(LDFLAGS)'
		

build: lowPrior highPrior

run: build
	@$(call msg,Running the test ...)
	@bash -c 'source ${ONEAPI_ROOT}/setvars.sh --force 2> /dev/null && \
		./memEviction_gpuTestTool.sh'

clean:
	@rm -rf lowPrior highPrior

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

