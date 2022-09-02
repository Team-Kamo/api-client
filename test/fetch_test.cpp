#include "include/internal/fetch.h"

#include <gtest/gtest.h>

#include <iostream>

#include "./mock/mock_http_client.h"
#include "include/error_code.h"

namespace octane::internal {
  /**
   * @brief
   * HttpClientの初期化に成功したときにFetchの初期化も成功するかどうかをテストする。
   *
   */
  TEST(FetchTest, InitOk) {
    octane::test::MockHttpClient mockHttpClient;
    EXPECT_CALL(mockHttpClient, init())
      .Times(1)
      .WillOnce(testing::Return(ok()));

    Fetch fetch("mock", "http://localhost:3000", "/api/v1", &mockHttpClient);
    EXPECT_TRUE(fetch.init());
  }
  /**
   * @brief
   * HttpClientの初期化に失敗したときにFetchの初期化も失敗するかどうかをテストする。
   */
  TEST(FetchTest, InitError) {
    octane::test::MockHttpClient mockHttpClient;
    EXPECT_CALL(mockHttpClient, init())
      .Times(1)
      .WillOnce(testing::Return(makeError(ERR_CURL_INITIALIZATION_FAILED, "")));

    Fetch fetch("mock", "http://localhost:3000", "/api/v1", &mockHttpClient);
    auto result = fetch.init();
    EXPECT_FALSE(result);
    EXPECT_EQ(result.err().code, ERR_CURL_INITIALIZATION_FAILED);
  }
  /**
   * @brief
   * HttpClientが期待通りに動作するときFetchが正常に動作することをテストする。
   *
   */
  TEST(FetchTest, HealthHealthy) {
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
  /**
   * @brief
   * レスポンスがapplication/jsonであるにもかかわらず正常なJSON出なかったときにエラーを返すかテストする。
   *
   */
  TEST(FetchTest, ExpectAnErrorWhenInvalidJsonIsResponded) {
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
    for (auto c : "I am not a JSON!!!!") {
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
    EXPECT_FALSE(response);
    EXPECT_EQ(response.err().code, ERR_JSON_PARSE_FAILED);
  }
  /**
   * @brief 3xx番台が返されたときにリダイレクト処理をするかテストをする。
   *
   */
  TEST(FetchTest, Redirect) {
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
    HttpResponse httpResponse{
      .statusCode  = 301,
      .statusLine  = "HTTP/2 301 Moved Permanently",
      .version     = HttpVersion::Http2,
      .headerField = { { "Location", "https://www.google.com" } },
      .body        = {},
    };

    HttpRequest httpRequest2{
      .method      = HttpMethod::Get,
      .version     = HttpVersion::Http2,
      .uri         = "/",
      .headerField = { { "X-Octane-API-Token", "mock" } },
      .body        = {},
    };
    std::vector<std::uint8_t> body;
    for (auto c : "<!doctype html><html></html>") {
      body.push_back(c);
    }
    HttpResponse httpResponse2{
      .statusCode  = 200,
      .statusLine  = "HTTP/2 200 OK",
      .version     = HttpVersion::Http2,
      .headerField = { { "Content-Type", "text/html" } },
      .body        = body,
    };

    EXPECT_CALL(
      mockHttpClient,
      request(std::string_view("https://www.google.com"), httpRequest2))
      .Times(1)
      .WillOnce(testing::Return(ok(httpResponse2)));
    EXPECT_CALL(mockHttpClient,
                request(std::string_view("http://localhost:3000"), httpRequest))
      .Times(1)
      .WillOnce(testing::Return(ok(httpResponse)));

    Fetch fetch("mock", "http://localhost:3000", "/api/v1", &mockHttpClient);
    EXPECT_TRUE(fetch.init());

    auto response = fetch.request(HttpMethod::Get, "/health");
    EXPECT_TRUE(response);
    EXPECT_EQ(response.get().statusCode, 200);
    EXPECT_EQ(response.get().mime, "text/html");
  }
} // namespace octane::internal