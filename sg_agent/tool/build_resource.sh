#!/bin/sh
	mkdir -p /octo/ns/sg_agent
	cd ../conf
	cp -rf idc.xml sg_agent_whitelist.xml \
	conf.json sg_agent_mutable.xml log4cplus.conf  /octo/ns/sg_agent
	cd ../bin
	cp -rf ServiceAgent /octo/ns/sg_agent
