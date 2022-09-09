#!/bin/sh
cd `dirname $0`
cmake -B build -DOCTANE_API_CLIENT_ENABLE_TESTING=ON
cmake --build build && cd ./build && ctest --rerun-failed --output-on-failure
