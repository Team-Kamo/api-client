cmake_minimum_required(VERSION 3.21)

function(make_test name)
  add_executable(
    ${name}
    ${name}.cpp
  )
  target_link_libraries(
    ${name}
    GTest::gtest_main
    GTest::gmock
    octane_api_client
  )
  target_include_directories(${name} PRIVATE ${OCTANE_API_CLIENT_INCLUDE_DIRS})
  gtest_discover_tests(${name})
endfunction()

make_test(api_client_test)
make_test(fetch_test)
make_test(http_client_test)
make_test(hash_test)
make_test(api_bridge_test)
