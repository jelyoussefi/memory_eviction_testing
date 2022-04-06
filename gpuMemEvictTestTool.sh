#!/bin/bash

./kernelCompiler

./gpuMemEvictTestTool -m $1 -t 60 &
lwpPid=$!

trap "kill ${lwpPid[@]}" SIGINT

sleep 15

echo  -e "$(tput setaf 3)\n\tSuspending Process $(tput sgr 0)"$lwpPid ;
kill -STOP $lwpPid

./gpuMemEvictTestTool -m $2 -t 20 -h

echo  -e "$(tput setaf 3)\n\tResuming Process $(tput sgr 0)"$lwpPid ;
kill -CONT $lwpPid

wait $lwpPid

