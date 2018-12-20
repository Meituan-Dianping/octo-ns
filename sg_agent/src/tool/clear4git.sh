#!/usr/bin/env bash
for module_path in src/mns src/test src/util
    do
      for file_path in cmake_install.cmake CMakeCache.txt CMakeFiles Makefile install_manifest.txt
        do
           rm -rf ${module_path}/${file_path}
        done;
    done;

#rm -rf sg_agent/build/ sg_agent/agent_bin/ sg_agent/lib/ sg_agent/bin/

find src/ -name .*.sw* -exec rm -rf {} \;
