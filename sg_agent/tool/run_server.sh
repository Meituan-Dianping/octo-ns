#!/bin/sh
export LD_LIBRARY_PATH

ulimit -n 1000000
ulimit -c 100000000

cd /octo/ns/sg_agent
nohup ./ServiceAgent &
