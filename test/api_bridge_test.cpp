#include "include/internal/api_bridge.h"

#include <gtest/gtest.h>
#include <rapidjson/error/en.h>

#include <string_view>

#include "./mock/mock_fetch.h"
#include "include/error_code.h"

namespace octane::internal {
  namespace {
    FetchResponse makeBinaryResponse(std::string_view data = "AAABBBCCC",
                                     int statusCode        = 200,
                                     std::string_view statusLine
                                     = "HTTP/2 200 OK",
                                     std::string_view mime = "image/png") {
      std::vector<uint8_t> body;
      body.resize(data.size());
      std::copy(data.begin(), data.end(), body.begin());
      return FetchResponse{
        .body       = body,
        .mime       = std::string(mime),
        .statusCode = statusCode,
        .statusLine = std::string(statusLine),
      };
    }
    rapidjson::Document makeJson(std::string_view data) {
      rapidjson::Document doc;
      doc.Parse(data.data(), data.size());
      if (doc.HasParseError()) {
        const auto message = rapidjson::GetParseError_En(doc.GetParseError());
        std::cout << message << std::endl;
        std::abort();
      }
      return doc;
    }
    FetchResponse makeJsonResponse(std::string_view data,
                                   int statusCode = 200,
                                   std::string_view statusLine
                                   = "HTTP/2 200 OK",
                                   std::string_view mime = "application/json") {
      return FetchResponse{
        .body       = makeJson(data),
        .mime       = std::string(mime),
        .statusCode = statusCode,
        .statusLine = std::string(statusLine),
      };
    }
  } // namespace
  /**
   * @brief
   * Fetchの初期化に成功した時にApiBridgeの初期化に成功するかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, InitOk) {
    test::MockFetch mockFetch;
    EXPECT_CALL(mockFetch, init()).Times(1).WillOnce(testing::Return(ok()));
    ApiBridge apiBridge(&mockFetch);
    EXPECT_TRUE(apiBridge.init());
  }
  /**
   * @brief
   * Fetchの初期化に失敗した時にApiBridgeの初期化に失敗するかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, InitError) {
    test::MockFetch mockFetch;
    EXPECT_CALL(mockFetch, init())
      .Times(1)
      .WillOnce(testing::Return(makeError(ERR_CURL_INITIALIZATION_FAILED, "")));
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.init();
    EXPECT_FALSE(result);
    EXPECT_EQ(result.err().code, ERR_CURL_INITIALIZATION_FAILED);
  }
  /**
   * @brief
   * healthGetにおいてFetchがHealthResultをhealthyとして返すときにApiBridgeがそれをそのまま返してくれるかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, healthGetOkHealthy) {
    test::MockFetch mockFetch;
    EXPECT_CALL(mockFetch, request(HttpMethod::Get, "/health"))
      .Times(1)
      .WillOnce(testing::Return(ok(makeJsonResponse(
        R"(
          {
            "health": "healthy",
            "message": ""
          }
        )"))));
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.healthGet();
    EXPECT_TRUE(result);
    EXPECT_EQ(result.get(),
              (HealthResult{
                .health  = Health::Healthy,
                .message = "",
              }));
  }
  /**
   * @brief
   * healthGetにおいてFetchがHealthResultをdegradedとして返すときにApiBridgeがそれをそのまま返してくれるかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, healthGetOkDegraded) {
    test::MockFetch mockFetch;
    EXPECT_CALL(mockFetch, request(HttpMethod::Get, "/health"))
      .Times(1)
      .WillOnce(testing::Return(ok(makeJsonResponse(
        R"(
          {
            "health": "degraded",
            "message": "server degraded gomennasai"
          }
        )"))));
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.healthGet();
    EXPECT_TRUE(result);
    EXPECT_EQ(result.get(),
              (HealthResult{
                .health  = Health::Degraded,
                .message = "server degraded gomennasai",
              }));
  }
  /**
   * @brief
   * healthGetにおいてFetchがHealthResultをfaultyとして返すときにApiBridgeがそれをそのまま返してくれるかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, healthGetOkFaulty) {
    test::MockFetch mockFetch;
    EXPECT_CALL(mockFetch, request(HttpMethod::Get, "/health"))
      .Times(1)
      .WillOnce(testing::Return(ok(makeJsonResponse(
        R"(
          {
            "health": "faulty",
            "message": "server faulty gomennasai"
          }
        )"))));
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.healthGet();
    EXPECT_TRUE(result);
    EXPECT_EQ(result.get(),
              (HealthResult{
                .health  = Health::Faulty,
                .message = "server faulty gomennasai",
              }));
  }
  /**
   * @brief
   * healthGetにおいてFetchがjsonのパースで失敗した時にApiBridgeがエラーを返してくれるかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, healthGetErrorJson) {
    test::MockFetch mockFetch;
    EXPECT_CALL(mockFetch, request(HttpMethod::Get, "/health"))
      .Times(1)
      .WillOnce(testing::Return(makeError(ERR_JSON_PARSE_FAILED, "")));
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.healthGet();
    EXPECT_FALSE(result);
    EXPECT_EQ(result.err().code, ERR_JSON_PARSE_FAILED);
  }
  /**
   * @brief
   * healthGetにおいてFetchがレスポンスの誤りで失敗した時にApiBridgeがエラーを返してくれるかどうかをテストする。これはjsonのメンバに誤りがある場合をテストしている。
   *
   */
  TEST(ApiBridgeTest, healthGetErrorResponse) {
    test::MockFetch mockFetch;
    EXPECT_CALL(mockFetch, request(HttpMethod::Get, "/health"))
      .Times(1)
      .WillOnce(testing::Return(ok(makeJsonResponse(
        R"(
          {
            "healthiest": "degraded",
            "message": "server degraded gomennasai"
          }
        )"))));
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.healthGet();
    EXPECT_FALSE(result);
    EXPECT_EQ(result.err().code, ERR_INVALID_RESPONSE);
  }
  /**
   * @brief
   * healthGetにおいてFetchがcURLの接続に失敗した時にApiBridgeがエラーを返してくれるかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, healthGetErrorCurl) {
    test::MockFetch mockFetch;
    EXPECT_CALL(mockFetch, request(HttpMethod::Get, "/health"))
      .Times(1)
      .WillOnce(testing::Return(makeError(ERR_CURL_CONNECTION_FAILED, "")));
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.healthGet();
    EXPECT_FALSE(result);
    EXPECT_EQ(result.err().code, ERR_CURL_CONNECTION_FAILED);
  }
  /**
   * @brief
   * healthGetにおいてFetchが2xx以外のステータスコードを返すときにサーバからもらうエラーレスポンスをそのまま返してくれるかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, healthGetErr2xxOk) {
    test::MockFetch mockFetch;
    EXPECT_CALL(mockFetch, request(HttpMethod::Get, "/health"))
      .Times(1)
      .WillOnce(
        testing::Return(ok(makeJsonResponse(R"(
        {
          "code": "ERR_BAD_REQUEST",
          "reason": ""
          }
        )",
                                            400,
                                            "HTTP/2 400 Bad Request"))));
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.healthGet();
    EXPECT_FALSE(result);
    EXPECT_EQ(result.err().code, "ERR_BAD_REQUEST");
  }
  /**
   * @brief
   * healthGetにおいてFetchが2xx以外のステータスコードを返すときにサーバからもらうエラーレスポンスに誤りがある場合に、そのレスポンス自身が誤りであることをApiBridge返すかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, healthGetErr2xxErr) {
    test::MockFetch mockFetch;
    EXPECT_CALL(mockFetch, request(HttpMethod::Get, "/health"))
      .Times(1)
      .WillOnce(
        testing::Return(ok(makeJsonResponse(R"(
        {
          "codeforces": "ERR_BAD_REQUEST",
          "reason": ""
          }
        )",
                                            400,
                                            "HTTP/2 400 Bad Request"))));
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.healthGet();
    EXPECT_FALSE(result);
    EXPECT_EQ(result.err().code, ERR_INVALID_RESPONSE);
  }
  /**
   * @brief
   * roomPostにおいてFetchが成功し、idを返す時にApiBridgeがそれをそのまま返してくれるかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, roomPostOk) {
    test::MockFetch mockFetch;
    std::string name = "soon's room";
    std::string id   = "07040782538";
    auto json        = makeJson(R"({"name": "soon's room"})");
    EXPECT_CALL(
      mockFetch,
      request(HttpMethod::Post, "/room", testing::Eq(testing::ByRef(json))))
      .Times(1)
      .WillOnce(
        testing::Return(ok(makeJsonResponse(R"({"id": "07040782538"})"))));
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.roomPost(name);
    EXPECT_TRUE(result);
    EXPECT_EQ(result.get(), id);
  }
  /**
   * @brief
   * roomPostにおいてFetchがレスポンスの誤りで失敗した時にApiBridgeがエラーを返してくれるかどうかをテストする。これはjsonのメンバの型に誤りがある場合をテストしている。
   *
   */
  TEST(ApiBridgeTest, roomPostErrResponse) {
    test::MockFetch mockFetch;
    std::string name = "soon's room";
    auto json        = makeJson(R"({"name": "soon's room"})");
    EXPECT_CALL(
      mockFetch,
      request(HttpMethod::Post, "/room", testing::Eq(testing::ByRef(json))))
      .Times(1)
      .WillOnce(
        testing::Return(ok(makeJsonResponse(R"({"ideco": "07040782538"})"))));
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.roomPost(name);
    EXPECT_FALSE(result);
    EXPECT_EQ(result.err().code, ERR_INVALID_RESPONSE);
  }
  /**
   * @brief
   * roomPostにおいてFetchがjsonのパースで失敗した時にApiBridgeがエラーを返してくれるかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, roomPostErrJson) {
    test::MockFetch mockFetch;
    std::string name = "soon's room";
    auto json        = makeJson(R"({"name": "soon's room"})");
    EXPECT_CALL(
      mockFetch,
      request(HttpMethod::Post, "/room", testing::Eq(testing::ByRef(json))))
      .Times(1)
      .WillOnce(testing::Return(makeError(ERR_JSON_PARSE_FAILED, "")));
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.roomPost(name);
    EXPECT_FALSE(result);
    EXPECT_EQ(result.err().code, ERR_JSON_PARSE_FAILED);
  }
  /**
   * @brief
   * roomPostにおいてFetchがcURLの接続に失敗した時にApiBridgeがエラーを返してくれるかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, roomPostErrCurl) {
    test::MockFetch mockFetch;
    std::string name = "soon's room";
    auto json        = makeJson(R"({"name": "soon's room"})");
    EXPECT_CALL(
      mockFetch,
      request(HttpMethod::Post, "/room", testing::Eq(testing::ByRef(json))))
      .Times(1)
      .WillOnce(testing::Return(makeError(ERR_CURL_CONNECTION_FAILED, "")));
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.roomPost(name);
    EXPECT_FALSE(result);
    EXPECT_EQ(result.err().code, ERR_CURL_CONNECTION_FAILED);
  }
  /**
   * @brief
   * roomPostにおいてFetchが2xx以外のステータスコードを返すときにサーバからもらうエラーレスポンスをそのまま返してくれるかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, roomPostErr2xx) {
    test::MockFetch mockFetch;
    std::string name = "soon's room";
    auto json        = makeJson(R"({"name": "soon's room"})");
    EXPECT_CALL(
      mockFetch,
      request(HttpMethod::Post, "/room", testing::Eq(testing::ByRef(json))))
      .Times(1)
      .WillOnce(
        testing::Return(ok(makeJsonResponse(R"(
        {
          "code": "ERR_BAD_REQUEST",
          "reason": ""
          }
        )",
                                            400,
                                            "HTTP/2 400 Bad Request"))));
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.roomPost(name);
    EXPECT_FALSE(result);
    EXPECT_EQ(result.err().code, "ERR_BAD_REQUEST");
  }

} // namespace octane::internal
