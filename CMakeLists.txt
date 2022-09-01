cmake_minimum_required(VERSION 3.21)

project(octane_api_client CXX)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

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

add_definitions(-D_UNICODE -DUNICODE)

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

include_directories(${PROJECT_BINARY_DIR}/include)
link_directories(${PROJECT_BINARY_DIR}/lib)

if(WIN32)
  set(ZLIB zlibstaticd)
elseif(APPLE)
  set(ZLIB libz.a)
else(UNIX)
  set(ZLIB libz.a)
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

include_directories(${rapidjson_SOURCE_DIR}/include)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include_directories(${googletest_SOURCE_DIR}/googletest/include ${gmock_SOURCE_DIR}/include)

include(GoogleTest)

enable_testing()

include_directories(${SRC_DIR})

add_subdirectory(${SRC_DIR})
add_subdirectory(${TEST_DIR})
