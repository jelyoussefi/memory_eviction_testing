#!/bin/bash

./gpuMemEvictTestTool -m $1 -t 60 &
lwpPid=$!

trap "kill ${lwpPid[@]}" SIGINT

sleep 15

kill -STOP $lwpPid

./gpuMemEvictTestTool -m $2 -t 20 -h

kill -STOP $lwpPid

wait $lwpPid

