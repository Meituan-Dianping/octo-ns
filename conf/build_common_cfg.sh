#!/bin/sh
	mkdir -p /data/webapps
	mkdir -p /octo/ns/
	cp -rf octo.cfg /data/webapps
	cp -rf idc.xml /octo/ns/
	echo "you have create the octo.cfg dir(/data/webapps) and idc.xml dir(/octo/ns/sg_agent)"
