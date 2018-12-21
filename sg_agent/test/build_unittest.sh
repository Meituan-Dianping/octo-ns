#!/bin/sh
cmake -DCMAKE_BUILD_TYPE=debug ../sg_agent/CMakeLists.txt
cd test
make -j8

mv -f ../../bin/unittest ./
