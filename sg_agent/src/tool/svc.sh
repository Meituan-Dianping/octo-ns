#!/bin/sh
DIR=`pwd`
BIN=$DIR"/sg_agent"

for pid in $(ps -ef | grep $BIN | grep -v grep | awk '{print $2}')
do
    kill $pid
done

folder="/opt/meituan/apps/sg_agent/sg_agent_bak"
if [ ! -x "$folder" ]; then 
    mkdir "$folder"
fi

