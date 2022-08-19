#!/bin/bash

if [ "$#" -ne 1 ]
then
  echo "usage : $0 <pid>"
  exit 1
fi

procPid=$!

trap "kill -STOP ${procPid[@]}"  SIGUSR1
trap "kill -CONT ${procPid[@]}"  SIGUSR2

wait $procPid
