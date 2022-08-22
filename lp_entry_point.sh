#!/bin/bash

source ~/.bashrc

cd ./apps/test/

./kernelCompiler 
./gpuMemEvictTestTool -m ${LP_MEM_RATIO} &
procPid=$!

. /usr/bin/suspend_resume_manager.sh $procPid


