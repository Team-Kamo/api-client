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

target_compile_definitions(octane_api_client PUBLIC LIBARCHIVE_STATIC)

target_include_directories(octane_api_client PUBLIC ${OCTANE_API_CLIENT_INCLUDE_DIRS})

target_link_libraries(octane_api_client PRIVATE OpenSSL::SSL CURL::libcurl ${ZLIB} ${LZMA} cryptopp::cryptopp archive_static)

if(WIN32)
  add_custom_command(
    TARGET octane_api_client PRE_LINK COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/../lib/liblzma.lib ${CMAKE_BINARY_DIR}/lib/liblzma.lib
  )
  add_custom_command(
    TARGET octane_api_client PRE_LINK COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/../bin/liblzma.dll ${CMAKE_BINARY_DIR}/bin/liblzma.dll
  )
endif()
