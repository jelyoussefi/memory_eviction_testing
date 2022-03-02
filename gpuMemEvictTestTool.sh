#!/bin/bash

./gpuMemEvictTestTool -m $1 -t 60 &
lwpPid=$!

trap "kill ${lwpPid[@]}" SIGINT

sleep 15

./gpuMemEvictTestTool -m $2 -t 20 -p $lwpPid


wait $lwpPid

