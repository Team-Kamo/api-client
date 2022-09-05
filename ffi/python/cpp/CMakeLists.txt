cmake_minimum_required(VERSION 3.21)

add_library(
  dynoctane SHARED
  api_entry.cpp
)

target_include_directories(dynoctane PUBLIC ${OCTANE_API_CLIENT_INCLUDE_DIRS})

target_link_libraries(dynoctane PRIVATE octane_api_client)

set(DYNOCTANE_API_CLIENT_OUTPUT ${ROOT_DIR}/ffi/python/python)

add_custom_command(TARGET dynoctane POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:dynoctane> ${DYNOCTANE_API_CLIENT_OUTPUT}
)

