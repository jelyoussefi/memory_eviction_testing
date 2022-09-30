#----------------------------------------------------------------------------------------------------------------------
# Flags
#----------------------------------------------------------------------------------------------------------------------
SHELL:=/bin/bash

export TERM=xterm

LP_MEM_RATIO ?= 1.0 
HP_MEM_RATIO ?= 1.0 
HP_DURATION ?= 20


#----------------------------------------------------------------------------------------------------------------------
# Targets
#----------------------------------------------------------------------------------------------------------------------
default: run 

build:
	@$(call msg,Building the docker images ...)	
	@COMPOSE_DOCKER_CLI_BUILD=1 DOCKER_BUILDKIT=1  docker-compose build
	
run: 
	@$(call msg,Running the gpuMemEvictTestTool application ...)
	@docker-compose down
	@docker-compose up -d prometheus  grafana memory_monitoring 
	#@sudo ./gpuMemEvictTestTool.sh ${LP_MEM_RATIO} ${HP_MEM_RATIO} ${HP_DURATION}

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

