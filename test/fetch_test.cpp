#include "include/internal/fetch.h"

#include <gtest/gtest.h>

#include <iostream>

#include "./mock/mock_http_client.h"
#include "include/error_code.h"

namespace octane::internal {
  TEST(FetchTest, InitOkTest) {
    octane::test::MockHttpClient mockHttpClient;
    EXPECT_CALL(mockHttpClient, init())
      .Times(1)
      .WillOnce(testing::Return(ok()));

    Fetch fetch("mock", "http://localhost:3000", "/api/v1", &mockHttpClient);
    EXPECT_TRUE(fetch.init());
  }
  TEST(FetchTest, InitErrorTest) {
    octane::test::MockHttpClient mockHttpClient;
    EXPECT_CALL(mockHttpClient, init())
      .Times(1)
      .WillOnce(testing::Return(makeError(ERR_CURL_INITIALIZATION_FAILED, "")));

    Fetch fetch("mock", "http://localhost:3000", "/api/v1", &mockHttpClient);
    auto result = fetch.init();
    EXPECT_FALSE(result);
    EXPECT_EQ(result.err().code, ERR_CURL_INITIALIZATION_FAILED);
  }

  TEST(FetchTest, HealthHealthyTest) {
    octane::test::MockHttpClient mockHttpClient;
    EXPECT_CALL(mockHttpClient, init())
      .Times(1)
      .WillOnce(testing::Return(ok()));

    HttpRequest httpRequest{
      .method      = HttpMethod::Get,
      .version     = HttpVersion::Http2,
      .uri         = "/api/v1/health",
      .headerField = { { "X-Octane-API-Token", "mock" } },
      .body        = {},
    };
    std::vector<std::uint8_t> body;
    for (auto c : "{\"health\": \"healthy\"}") {
      body.push_back(c);
    }
    HttpResponse httpResponse{
      .statusCode  = 200,
      .statusLine  = "HTTP/2 200 OK",
      .version     = HttpVersion::Http2,
      .headerField = { { "Content-Type", "application/json" } },
      .body        = std::move(body),
    };
    EXPECT_CALL(mockHttpClient,
                request(std::string_view("http://localhost:3000"), httpRequest))
      .Times(1)
      .WillOnce(testing::Return(ok(httpResponse)));

    Fetch fetch("mock", "http://localhost:3000", "/api/v1", &mockHttpClient);
    EXPECT_TRUE(fetch.init());

    auto response = fetch.request(HttpMethod::Get, "/health");
    EXPECT_TRUE(response);
    EXPECT_TRUE(
      std::holds_alternative<rapidjson::Document>(response.get().body));

    auto& json = std::get<rapidjson::Document>(response.get().body);
    EXPECT_TRUE(json.HasMember("health"));
    EXPECT_EQ(json["health"], "healthy");
  }

  TEST(FetchTest, HealthDegradedTest) {
    octane::test::MockHttpClient mockHttpClient;
    EXPECT_CALL(mockHttpClient, init())
      .Times(1)
      .WillOnce(testing::Return(ok()));

    HttpRequest httpRequest{
      .method      = HttpMethod::Get,
      .version     = HttpVersion::Http2,
      .uri         = "/api/v1/health",
      .headerField = { { "X-Octane-API-Token", "mock" } },
      .body        = {},
    };
    std::vector<std::uint8_t> body;
    for (auto c : "{\"health\": \"degraded\"}") {
      body.push_back(c);
    }
    HttpResponse httpResponse{
      .statusCode  = 200,
      .statusLine  = "HTTP/2 200 OK",
      .version     = HttpVersion::Http2,
      .headerField = { { "Content-Type", "application/json" } },
      .body        = std::move(body),
    };
    EXPECT_CALL(mockHttpClient,
                request(std::string_view("http://localhost:3000"), httpRequest))
      .Times(1)
      .WillOnce(testing::Return(ok(httpResponse)));

    Fetch fetch("mock", "http://localhost:3000", "/api/v1", &mockHttpClient);
    EXPECT_TRUE(fetch.init());

    auto response = fetch.request(HttpMethod::Get, "/health");
    EXPECT_TRUE(response);
    EXPECT_TRUE(
      std::holds_alternative<rapidjson::Document>(response.get().body));

    auto& json = std::get<rapidjson::Document>(response.get().body);
    EXPECT_TRUE(json.HasMember("health"));
    EXPECT_EQ(json["health"], "degraded");
  }
} // namespace octane::internal