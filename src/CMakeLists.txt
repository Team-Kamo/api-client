cmake_minimum_required(VERSION 3.21)

add_library(
  octane_api_client
  cpp/api_client.cpp
  cpp/error_response.cpp
  cpp/api_result_types.cpp
  cpp/internal/fetch.cpp
  cpp/internal/http_client.cpp
  cpp/internal/api_bridge.cpp
  cpp/internal/hash.cpp
  cpp/internal/multi_file.cpp
)

target_include_directories(octane_api_client PUBLIC ${OCTANE_API_CLIENT_INCLUDE_DIRS})

target_link_libraries(octane_api_client PRIVATE OpenSSL::SSL CURL::libcurl zlib liblzma cryptopp::cryptopp archive)
