#!/bin/sh
# shellcheck shell=sh
# shellcheck disable=SC2128,SC2009
first1=$(date +"%s")
#echo "start_time: "$start1

echo "launch gpuTestTool with unique in the backgroud; save its pid"
echo ""
./gpuMemEvictTestTool -m 0.8 -t 120 &
thepid=$!

sleep 15


./gpuMemEvictTestTool -m 0.6 -t 20 -p $thepid


