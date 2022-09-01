cmake_minimum_required(VERSION 3.21)

add_library(
  octane
  cpp/api_client.cpp
  cpp/error_response.cpp
  cpp/internal/fetch.cpp
  cpp/internal/http_client.cpp
)

add_dependencies(octane zlib)
target_link_libraries(octane PRIVATE OpenSSL::SSL CURL::libcurl ${ZLIB})
