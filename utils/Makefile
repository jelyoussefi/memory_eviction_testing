#----------------------------------------------------------------------------------------------------------------------
# Flags
#----------------------------------------------------------------------------------------------------------------------
SHELL:=/bin/bash

CURRENT_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

export TERM=xterm

CXX_COMPILER=g++
CXXFLAGS=-g -Wno-c++20-extensions -Wno-deprecated-declarations -Wno-return-type 
LDFLAGS= -lpthread -lstdc++fs -lprometheus-cpp-core -lprometheus-cpp-pull


DOCKER_BASE_IMAGE ?= ge/intel/sles_dpcpp_compiler
DOCKER_IMAGE_NAME ?= memory_monitor_image


#----------------------------------------------------------------------------------------------------------------------
# Targets
#----------------------------------------------------------------------------------------------------------------------
default: build 
.PHONY:  grafana

.PHONY: prometheus-cpp memMonitor

prometheus-cpp:
	@$(call msg,Building  prometheus-cpp ...)
	@cd /tmp && rm -rf prometheus-cpp && git clone https://github.com/jupp0r/prometheus-cpp && \
		cd prometheus-cpp && git submodule init && git submodule update && \
		mkdir build && cd build && CXX=g++ \
		cmake .. -DBUILD_SHARED_LIBS=ON -DENABLE_PUSH=OFF -DENABLE_COMPRESSION=OFF .. && \
		make && sudo make install

memMonitor: 
	@$(call msg,Building the  memory monitoring application   ...)
	@$(CXX_COMPILER) $(CXXFLAGS) $@.cpp -o $@ $(LDFLAGS) && cp $@ /usr/bin/


build: prometheus-cpp memMonitor

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

