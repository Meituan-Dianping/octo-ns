#!/bin/sh
sh src/tool/clear4git.sh
cmake src/CMakeLists.txt
cd src
make -j8
make install

