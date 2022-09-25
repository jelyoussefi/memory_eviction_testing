#!/bin/bash

./kernelCompiler

./gpuMemEvictTestTool -m $1 -b 512 -t 15 &
lwpPid=$!

trap "kill ${lwpPid[@]}" SIGINT

sleep 5

echo  -e "$(tput setaf 3)\n\tSuspending Process $(tput sgr 0)"$lwpPid ;
kill -STOP $lwpPid

./gpuMemEvictTestTool -m $2 -b 512 -t 5 -h

echo  -e "$(tput setaf 3)\n\tResuming Process $(tput sgr 0)"$lwpPid ;
kill -CONT $lwpPid

wait $lwpPid

