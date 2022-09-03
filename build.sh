#!/bin/sh
cd `dirname $0`
cmake -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc -B build -DOCTANE_API_CLIENT_ENABLE_TESTING=ON
cmake --build build && cd ./build && ctest
