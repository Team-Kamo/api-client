cd /d %~dp0
cmake -B build -DOCTANE_API_CLIENT_ENABLE_TESTING=ON
cmake --build build && cd ./build && ctest