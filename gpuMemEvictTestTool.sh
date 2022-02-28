#!/bin/bash

./gpuMemEvictTestTool -m $1 -t 50 &
thepid=$!

sleep 15

./gpuMemEvictTestTool -m $2 -t 20 -p $thepid


