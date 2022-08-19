#!/bin/bash

source ~/.bashrc

cd ./apps/test/

./kernelCompiler 
./gpuMemEvictTestTool -m 0.8 &
procPid=$!

. /usr/bin/suspend_resume_manager.sh $procPid


