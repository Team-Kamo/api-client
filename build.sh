#!/bin/sh
cd `dirname $0`
cmake -B build
cmake --build build && cd ./build && ctest