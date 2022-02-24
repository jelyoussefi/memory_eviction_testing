#!/bin/sh
# shellcheck shell=sh
# shellcheck disable=SC2128,SC2009
first1=$(date +"%s")
#echo "start_time: "$start1

echo "launch gpuTestTool with unique in the backgroud; save its pid"
./lowPrior -d 2 -v -M UNIQUE&
thepid=$!

echo "    pid: "$thepid

echo "suspend the gpuTestTool after 10 sec"
sleep 10
kill -STOP $thepid
first2=$(date +"%s")
sleep 3

echo "run vector-add with 2 kernels"
#echo "start_time 2: "$start2
./highPrior $1

echo "resume the gpuTestTool & wait for it to finish"
second1=$(date +"%s")
kill -CONT $thepid
wait $thepid 
second2=$(date +"%s")

echo "***testing completed****"

elapset1=$(($first2-$first1))
elapset2=$(($second2-$second1))
gpuTestUsed=$(($elapset2+$elapset1))
echo "elapsed time: "$gpuTestUsed
