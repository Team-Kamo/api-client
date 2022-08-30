cmake_minimum_required(VERSION 3.21)

add_executable(
  api_client_test
  api_client_test.cpp
)
target_link_libraries(
  api_client_test
  GTest::gtest_main
  liboctane
)

gtest_discover_tests(api_client_test)
