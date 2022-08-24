#!/bin/bash

if [ "$#" -ne 1 ]
then
  echo "usage : $0 <pid>"
  exit 1
fi

procPid=$1

function suspend_handler() {
  kill -STOP ${procPid[@]}
  suspend_resume_prom -s
}

function resume_handler() {
  kill -CONT ${procPid[@]}
  suspend_resume_prom -r
}


trap suspend_handler  SIGUSR1
trap resume_handler   SIGUSR2

while true
do
	wait $procPid
done
