#!/usr/bin/env bash

rm -rf ../cmake_install.cmake ../CMakeCache.txt ../CMakeFiles ../Makefile ../build/ ../install_manifest.txt
rm -rf ../src/cmake_install.cmake ../src/CMakeCache.txt ../src/CMakeFiles ../src/Makefile ../src/gen-cpp
rm -rf ../src/tests/cmake_install.cmake ../src/tests/CMakeCache.txt ../src/tests/CMakeFiles ../src/tests/Makefile
rm -rf ../example/cmake_install.cmake ../example/CMakeCache.txt ../example/CMakeFiles ../example/Makefile

find ../ -name .*.sw* -exec rm -rf {} \;
