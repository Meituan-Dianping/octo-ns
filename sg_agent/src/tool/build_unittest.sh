#!/bin/sh
#sh tool/clear4git.sh
cmake -DCMAKE_BUILD_TYPE=debug src/CMakeLists.txt
cd src/test
make -j8
make install
