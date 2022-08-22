#!/bin/bash

source ~/.bashrc

cd ./apps/test/

./kernelCompiler 
./gpuMemEvictTestTool -m ${HP_MEM_RATIO} -t ${HP_DURATION} -h

