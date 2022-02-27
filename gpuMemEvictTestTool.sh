#!/bin/sh

./gpuMemEvictTestTool -m 0.8 -t 60 &
thepid=$!

sleep 15

./gpuMemEvictTestTool -m 0.6 -t 20 -p $thepid


