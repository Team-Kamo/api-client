#include "include/fetch.h"

#include <gtest/gtest.h>

#include <iostream>

using namespace octane::internal;

TEST(FetchTest, HealthTest) {
  Fetch fetch("mock", "http://localhost:3000", "/api/v1");
  auto response = fetch.request(HttpMethod::Get, "/health");
  ASSERT_TRUE(response);
  if (response) {
    // curl -vLH "X-Octane-API-Token:mock" http://localhost:3000/api/v1/health
    auto& health = response.get();
    if(std::holds_alternative<rapidjson::Document>(health)) {
      std::cout << "JSON" << std::endl;
      auto& json = std::get<rapidjson::Document>(health);
      std::cout << json["health"].GetString() << std::endl;
      std::cout << json["message"].GetString() << std::endl;
    } else {
      std::cout << "BINARY" << std::endl;
    }
  } else {
    std::cout << response.err().code << std::endl;
    std::cout << response.err().reason << std::endl;
  }
}