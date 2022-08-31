cmake_minimum_required(VERSION 3.21)

add_library(
  octane
  cpp/api_client.cpp
  cpp/fetch.cpp
)

add_dependencies(octane zlib)
target_link_libraries(octane OpenSSL::SSL CURL::libcurl ${ZLIB})
