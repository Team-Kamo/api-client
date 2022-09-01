cmake_minimum_required(VERSION 3.21)

add_executable(
  api_client_test
  api_client_test.cpp
)
target_link_libraries(
  api_client_test
  GTest::gtest_main
  GTest::gmock
  octane_api_client
)
target_include_directories(api_client_test PRIVATE ${OCTANE_API_CLIENT_INCLUDE_DIRS})
gtest_discover_tests(api_client_test)

add_executable(
  fetch_test
  fetch_test.cpp
)
target_link_libraries(
  fetch_test
  GTest::gtest_main
  GTest::gmock
  octane_api_client
)
target_include_directories(fetch_test PRIVATE ${OCTANE_API_CLIENT_INCLUDE_DIRS})
gtest_discover_tests(fetch_test)

add_executable(
  http_client_test
  http_client_test.cpp
)
target_link_libraries(
  http_client_test
  GTest::gtest_main
  GTest::gmock
  octane_api_client
)
target_include_directories(http_client_test PRIVATE ${OCTANE_API_CLIENT_INCLUDE_DIRS})
gtest_discover_tests(http_client_test)
