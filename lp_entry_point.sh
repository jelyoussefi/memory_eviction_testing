#!/bin/bash

source ~/.bashrc

cd ./apps/test/


./oneAPIMemTest &

procPid=$!

. /usr/bin/suspend_resume_manager.sh $procPid


