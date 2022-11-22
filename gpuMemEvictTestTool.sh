#!/bin/bash


./oneAPIMemTest -m $1 -t 15 &
lwpPid=$!

trap "kill ${lwpPid[@]}" SIGINT

sleep 5

echo  -e "$(tput setaf 3)\n\tSuspending Process $(tput sgr 0)"$lwpPid ;
kill -STOP $lwpPid

./oneAPIMemTest -m $2  -t 5 -i 1 

echo  -e "$(tput setaf 3)\n\tResuming Process $(tput sgr 0)"$lwpPid ;
kill -CONT $lwpPid

wait $lwpPid

