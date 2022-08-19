#!/bin/bash

source ~/.bashrc

cd ./apps/test/

./kernelCompiler 
./gpuMemEvictTestTool -m 0.8 -t 20 -h

