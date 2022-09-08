cmake_minimum_required(VERSION 3.21)

project(octane_api_client CXX)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/MP>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/MP>")

enable_language(CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-long-long -pedantic")
endif()

if(WIN32)
  set(CMAKE_SHARED_LINKER_FLAGS /MANIFEST:NO)
endif(WIN32)

add_definitions(-D_UNICODE -DUNICODE -DHAVE_LZMA_H)

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SRC_DIR ${ROOT_DIR}/src)
set(TEST_DIR ${ROOT_DIR}/test)

set(CMAKE_USE_OPENSSL)
include(FindOpenSSL)
include(FindCURL)

include(ExternalProject)

ExternalProject_Add(
  zlib
  GIT_REPOSITORY "https://github.com/madler/zlib"
  GIT_TAG "v1.2.12"
  PREFIX "${CMAKE_BINARY_DIR}/zlib"
  CMAKE_CACHE_ARGS
  "-DBUILD_SHARED_LIBS:BOOL=OFF"
  "-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}"
)

link_directories(${CMAKE_BINARY_DIR}/lib)

# cryptopp options
set(CRYPTOPP_BUILD_TESTING OFF)

# libarchive options
set(ENABLE_TEST OFF)
set(ENABLE_INSTALL OFF)

if(WIN32)
  set(ZLIB zlibstaticd)
  set(LZMALIB ${CMAKE_CURRENT_SOURCE_DIR}/lib/liblzma.lib)
  set(LZMABIN ${CMAKE_CURRENT_SOURCE_DIR}/bin/liblzma.dll)
elseif(APPLE)
  set(ZLIB libz.a)
  add_compile_definitions(CRYPTOPP_DISABLE_ASM)
  set(LZMALIB lzma)
elseif(UNIX)
  set(ZLIB libz.a)
  set(LZMALIB lzma)
endif()

include(FetchContent)

FetchContent_Declare(
  rapidjson
  GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
  GIT_TAG v1.1.0
)
FetchContent_GetProperties(rapidjson)

if(NOT rapidjson_POPULATED)
  FetchContent_Populate(rapidjson)
endif()

FetchContent_Declare(
  cryptopp-cmake
  GIT_REPOSITORY https://github.com/abdes/cryptopp-cmake.git
)

FetchContent_Declare(
  libarchive
  GIT_REPOSITORY https://github.com/libarchive/libarchive
  GIT_TAG v3.6.1
)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(cryptopp-cmake libarchive googletest)

include(GoogleTest)

set(
  OCTANE_API_CLIENT_INCLUDE_DIRS
  ${SRC_DIR}
  ${CMAKE_BINARY_DIR}/include
  ${rapidjson_SOURCE_DIR}/include
  ${cryptopp_SOURCE_DIR}
  ${libarchive_SOURCE_DIR}
  ${googletest_SOURCE_DIR}/googletest/include
  ${gmock_SOURCE_DIR}/include
)
set(
  OCTANE_API_CLIENT_LIB_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/lib
)
set(
  OCTANE_API_CLIENT_BIN_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/bin
)
set(
  OCTANE_API_CLIENT_LIBRARIES
  ${CMAKE_BINARY_DIR}/lib/liblzma.lib
)

add_subdirectory(${SRC_DIR})
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ffi/python/cpp)

if(OCTANE_API_CLIENT_ENABLE_TESTING)
  enable_testing()
  add_subdirectory(${TEST_DIR})
endif(OCTANE_API_CLIENT_ENABLE_TESTING)
