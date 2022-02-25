#!/bin/sh
# shellcheck shell=sh
# shellcheck disable=SC2128,SC2009
first1=$(date +"%s")
#echo "start_time: "$start1

echo "launch gpuTestTool with unique in the backgroud; save its pid"
./gpuMemEvictTestTool -l &
thepid=$!

sleep 10
echo "\n\tSuspending the low priority gpuTestTool after 10 sec"
kill -STOP $thepid
first2=$(date +"%s")
sleep 3

#echo "start_time 2: "$start2
./gpuMemEvictTestTool -n $1 

echo "\tResuming the low priority gpuTestTool & wait for it to finish"
second1=$(date +"%s")
kill -CONT $thepid
wait $thepid 
second2=$(date +"%s")


elapset1=$(($first2-$first1))
elapset2=$(($second2-$second1))
gpuTestUsed=$(($elapset2+$elapset1))
echo "elapsed time: "$gpuTestUsed
