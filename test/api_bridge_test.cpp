#include "include/internal/api_bridge.h"

#include <gtest/gtest.h>
#include <rapidjson/error/en.h>

#include <string_view>

#include "./mock/mock_fetch.h"
#include "include/error_code.h"

namespace octane::internal {
  namespace {
    std::string toString(const std::vector<std::uint8_t>& body) {
      std::string data;
      data.resize(body.size());
      std::copy(body.begin(), body.end(), data.begin());
      return data;
    }
    std::vector<std::uint8_t> toBinary(std::string_view str) {
      std::vector<std::uint8_t> data;
      data.resize(str.size());
      std::copy(str.begin(), str.end(), data.begin());
      return data;
    }
    FetchResponse makeEmptyResponse(int statusCode = 200,
                                    std::string_view statusLine
                                    = "HTTP/2 200 OK") {
      return FetchResponse{
        .statusCode = statusCode,
        .statusLine = std::string(statusLine),
      };
    }
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
    EXPECT_EQ(result.err().code, ERR_CURL_INITIALIZATION_FAILED)
      << result.err();
  }
  /**
   * @brief
   * healthGetにおいてFetchがHealthResultをhealthyとして返すときにApiBridgeがそれをそのまま返してくれるかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, healthGetOkHealthy) {
    test::MockFetch mockFetch;
    EXPECT_CALL(mockFetch,
                request(HttpMethod::Get, std::string_view("/health")))
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
    EXPECT_TRUE(result) << result.err();
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
    EXPECT_CALL(mockFetch,
                request(HttpMethod::Get, std::string_view("/health")))
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
    EXPECT_TRUE(result) << result.err();
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
    EXPECT_CALL(mockFetch,
                request(HttpMethod::Get, std::string_view("/health")))
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
    EXPECT_TRUE(result) << result.err();
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
    EXPECT_CALL(mockFetch,
                request(HttpMethod::Get, std::string_view("/health")))
      .Times(1)
      .WillOnce(testing::Return(makeError(ERR_JSON_PARSE_FAILED, "")));
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.healthGet();
    EXPECT_FALSE(result) << result.get();
    EXPECT_EQ(result.err().code, ERR_JSON_PARSE_FAILED) << result.err();
  }
  /**
   * @brief
   * healthGetにおいてFetchがレスポンスの誤りで失敗した時にApiBridgeがエラーを返してくれるかどうかをテストする。これはjsonのメンバに誤りがある場合をテストしている。
   *
   */
  TEST(ApiBridgeTest, healthGetErrorResponse) {
    test::MockFetch mockFetch;
    EXPECT_CALL(mockFetch,
                request(HttpMethod::Get, std::string_view("/health")))
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
    EXPECT_FALSE(result) << result.get();
    EXPECT_EQ(result.err().code, ERR_INVALID_RESPONSE) << result.err();
  }
  /**
   * @brief
   * healthGetにおいてFetchがcURLの接続に失敗した時にApiBridgeがエラーを返してくれるかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, healthGetErrorCurl) {
    test::MockFetch mockFetch;
    EXPECT_CALL(mockFetch,
                request(HttpMethod::Get, std::string_view("/health")))
      .Times(1)
      .WillOnce(testing::Return(makeError(ERR_CURL_CONNECTION_FAILED, "")));
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.healthGet();
    EXPECT_FALSE(result) << result.get();
    EXPECT_EQ(result.err().code, ERR_CURL_CONNECTION_FAILED) << result.err();
  }
  /**
   * @brief
   * healthGetにおいてFetchが2xx以外のステータスコードを返すときにサーバからもらうエラーレスポンスをそのまま返してくれるかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, healthGetErr2xxOk) {
    test::MockFetch mockFetch;
    EXPECT_CALL(mockFetch,
                request(HttpMethod::Get, std::string_view("/health")))
      .Times(1)
      .WillOnce(testing::Return(ok(makeJsonResponse(
        R"(
          {
            "code": "ERR_BAD_REQUEST",
            "reason": ""
          }
        )",
        400,
        "HTTP/2 400 Bad Request"))));
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.healthGet();
    EXPECT_FALSE(result) << result.get();
    EXPECT_EQ(result.err().code, "ERR_BAD_REQUEST") << result.err();
  }
  /**
   * @brief
   * healthGetにおいてFetchが2xx以外のステータスコードを返すときにサーバからもらうエラーレスポンスに誤りがある場合に、そのレスポンス自身が誤りであることをApiBridge返すかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, healthGetErr2xxErr) {
    test::MockFetch mockFetch;
    EXPECT_CALL(mockFetch,
                request(HttpMethod::Get, std::string_view("/health")))
      .Times(1)
      .WillOnce(testing::Return(ok(makeJsonResponse(
        R"(
          {
            "codeforce": "ERR_BAD_REQUEST",
            "reason": ""
          }
        )",
        400,
        "HTTP/2 400 Bad Request"))));
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.healthGet();
    EXPECT_FALSE(result) << result.get();
    EXPECT_EQ(result.err().code, ERR_INVALID_RESPONSE) << result.err();
  }
  /**
   * @brief
   * roomPostにおいてFetchが成功し、idを返す時にApiBridgeがそれをそのまま返してくれるかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, roomPostOk) {
    test::MockFetch mockFetch;
    std::string name = "soon's room";
    RoomId roomId{};
    std::uint64_t id = 7040782538;
    roomId.id        = id;
    auto json        = makeJson(R"({"name": "soon's room"})");
    EXPECT_CALL(mockFetch,
                request(HttpMethod::Post,
                        std::string_view("/room"),
                        testing::Eq(testing::ByRef(json))))
      .Times(1)
      .WillOnce(testing::Return(ok(makeJsonResponse(R"({"id": 7040782538})"))));
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.roomPost(name);
    EXPECT_TRUE(result) << result.err();
    EXPECT_EQ(result.get(), roomId);
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
    EXPECT_CALL(mockFetch,
                request(HttpMethod::Post,
                        std::string_view("/room"),
                        testing::Eq(testing::ByRef(json))))
      .Times(1)
      .WillOnce(
        testing::Return(ok(makeJsonResponse(R"({"ideco": 7040782538})"))));
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.roomPost(name);
    EXPECT_FALSE(result) << result.get();
    EXPECT_EQ(result.err().code, ERR_INVALID_RESPONSE) << result.err();
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
    EXPECT_CALL(mockFetch,
                request(HttpMethod::Post,
                        std::string_view("/room"),
                        testing::Eq(testing::ByRef(json))))
      .Times(1)
      .WillOnce(testing::Return(makeError(ERR_JSON_PARSE_FAILED, "")));
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.roomPost(name);
    EXPECT_FALSE(result) << result.get();
    EXPECT_EQ(result.err().code, ERR_JSON_PARSE_FAILED) << result.err();
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
    EXPECT_CALL(mockFetch,
                request(HttpMethod::Post,
                        std::string_view("/room"),
                        testing::Eq(testing::ByRef(json))))
      .Times(1)
      .WillOnce(testing::Return(makeError(ERR_CURL_CONNECTION_FAILED, "")));
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.roomPost(name);
    EXPECT_FALSE(result) << result.get();
    EXPECT_EQ(result.err().code, ERR_CURL_CONNECTION_FAILED) << result.err();
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
    EXPECT_CALL(mockFetch,
                request(HttpMethod::Post,
                        std::string_view("/room"),
                        testing::Eq(testing::ByRef(json))))
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
    EXPECT_FALSE(result) << result.get();
    EXPECT_EQ(result.err().code, "ERR_BAD_REQUEST") << result.err();
  }
  /**
   * @brief
   * roomIdGetにおいてFetchが成功し、ルームのステータスを返す時にApiBridgeがそれをそのまま返してくれるかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, roomIdGetOk) {
    test::MockFetch mockFetch;
    std::uint64_t id = 7040782538;
    auto url         = "/room/" + std::to_string(id);
    EXPECT_CALL(mockFetch, request(HttpMethod::Get, std::string_view(url)))
      .Times(1)
      .WillOnce(testing::Return(ok(makeJsonResponse(R"(
        {
          "devices": [{"name": "soon's thinkpad", "timestamp": 50220835}],
          "name" : "soon's super cool octane room",
          "id": 7040782538
          }
        )"))));
    RoomStatus roomStatus{};
    Device device{};
    device.name      = "soon's thinkpad";
    device.timestamp = 50220835;
    roomStatus.devices.push_back(device);
    roomStatus.name = "soon's super cool octane room";
    roomStatus.id   = id;
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.roomIdGet(id);
    EXPECT_TRUE(result) << result.err();
    EXPECT_EQ(result.get(), roomStatus) << result.get();
  }
  /**
   * @brief
   * roomIdGetにおいてにおいてFetchは成功したがレスポンスの誤りがあった時にApiBridgeがエラーを返してくれるかどうかをテストする。これはjsonのメンバに誤りがある場合をテストしている。
   *
   */
  TEST(ApiBridgeTest, roomIdGetErrResponse) {
    test::MockFetch mockFetch;
    std::uint64_t id = 7040782538;
    auto url         = "/room/" + std::to_string(id);
    EXPECT_CALL(mockFetch, request(HttpMethod::Get, std::string_view(url)))
      .Times(1)
      .WillOnce(testing::Return(ok(makeJsonResponse(R"(
        {
          "Devices": [{"name": "soon's thinkpad", "timestamp": 50220835}],
          "Name" : "soon's super cool octane room",
          "Id": 7040782538
          }
        )"))));
    RoomStatus roomStatus{};
    Device device{};
    device.name      = "soon's thinkpad";
    device.timestamp = 50220835;
    roomStatus.devices.push_back(device);
    roomStatus.name = "soon's super cool octane room";
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.roomIdGet(id);
    EXPECT_FALSE(result) << result.get();
    EXPECT_EQ(result.err().code, ERR_INVALID_RESPONSE) << result.err();
  }
  /**
   * @brief
   * roomIdGetにおいてFetchがjsonのパースで失敗した時にApiBridgeがエラーを返してくれるかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, roomIdGetErrJson) {
    test::MockFetch mockFetch;
    std::uint64_t id = 7040782538;
    auto url         = "/room/" + std::to_string(id);
    EXPECT_CALL(mockFetch, request(HttpMethod::Get, std::string_view(url)))
      .Times(1)
      .WillOnce(testing::Return(makeError(ERR_JSON_PARSE_FAILED, "")));
    RoomStatus roomStatus{};
    Device device{};
    device.name      = "soon's thinkpad";
    device.timestamp = 50220835;
    roomStatus.devices.push_back(device);
    roomStatus.name = "soon's super cool octane room";
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.roomIdGet(id);
    EXPECT_FALSE(result) << result.get();
    EXPECT_EQ(result.err().code, ERR_JSON_PARSE_FAILED) << result.err();
  }
  /**
   * @brief
   * roomIdGetにおいてFetchがcURLの接続に失敗した時にApiBridgeがエラーを返してくれるかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, roomIdGetErrCurl) {
    test::MockFetch mockFetch;
    std::uint64_t id = 7040782538;
    auto url         = "/room/" + std::to_string(id);
    EXPECT_CALL(mockFetch, request(HttpMethod::Get, std::string_view(url)))
      .Times(1)
      .WillOnce(testing::Return(makeError(ERR_CURL_CONNECTION_FAILED, "")));
    RoomStatus roomStatus{};
    Device device{};
    device.name      = "soon's thinkpad";
    device.timestamp = 50220835;
    roomStatus.devices.push_back(device);
    roomStatus.name = "soon's super cool octane room";
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.roomIdGet(id);
    EXPECT_FALSE(result) << result.get();
    EXPECT_EQ(result.err().code, ERR_CURL_CONNECTION_FAILED) << result.err();
  }
  /**
   * @brief
   * roomIdGetにおいてFetchが2xx以外のステータスコードを返すときにサーバからもらうエラーレスポンスをそのまま返してくれるかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, roomIdGet2xx) {
    test::MockFetch mockFetch;
    std::uint64_t id = 7040782538;
    auto url         = "/room/" + std::to_string(id);
    EXPECT_CALL(mockFetch, request(HttpMethod::Get, std::string_view(url)))
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
    auto result = apiBridge.roomIdGet(id);
    EXPECT_FALSE(result) << result.get();
    EXPECT_EQ(result.err().code, "ERR_BAD_REQUEST") << result.err();
  }
  /**
   * @brief
   * roomIdDeleteにおいてFetchが成功した時にApiBridgeが何も返さないかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, roomIdDeleteOk) {
    test::MockFetch mockFetch;
    std::uint64_t id = 7040782538;
    auto url         = "/room/" + std::to_string(id);
    EXPECT_CALL(mockFetch, request(HttpMethod::Delete, std::string_view(url)))
      .Times(1)
      .WillOnce(testing::Return(ok(makeEmptyResponse())));
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.roomIdDelete(id);
    EXPECT_TRUE(result) << result.err();
  }
  /**
   * @brief
   * roomIdDeleteにおいてFetchがcURLの接続に失敗した時にApiBridgeがエラーを返してくれるかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, roomIdDeleteErrCurl) {
    test::MockFetch mockFetch;
    std::uint64_t id = 7040782538;
    auto url         = "/room/" + std::to_string(id);
    EXPECT_CALL(mockFetch, request(HttpMethod::Delete, std::string_view(url)))
      .Times(1)
      .WillOnce(testing::Return(makeError(ERR_CURL_CONNECTION_FAILED, "")));
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.roomIdDelete(id);
    EXPECT_FALSE(result);
    EXPECT_EQ(result.err().code, ERR_CURL_CONNECTION_FAILED) << result.err();
  }
  /**
   * @brief
   * roomIdDeleteにおいてFetchが2xx以外のステータスコードを返すときにサーバからもらうエラーレスポンスをそのまま返してくれるかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, roomIdDelete2xx) {
    test::MockFetch mockFetch;
    std::uint64_t id = 7040782538;
    auto url         = "/room/" + std::to_string(id);
    EXPECT_CALL(mockFetch, request(HttpMethod::Delete, std::string_view(url)))
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
    auto result = apiBridge.roomIdDelete(id);
    EXPECT_FALSE(result);
    EXPECT_EQ(result.err().code, "ERR_BAD_REQUEST") << result.err();
  }
  /**
   * @brief
   * roomIdPostにおいてConnectリクエストを出し、Fetchが成功した時にApiBridgeが何も返さないかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, roomIdPostOkConnect) {
    test::MockFetch mockFetch;
    std::uint64_t id = 7040782538;
    std::string name
      = "soon's macbook air 13manyenguraidegakuwaridekaemashitaureshiine!";
    auto url  = "/room/" + std::to_string(id);
    auto json = makeJson(
      R"({"name": "soon's macbook air 13manyenguraidegakuwaridekaemashitaureshiine!", "request": "connect"})");
    EXPECT_CALL(mockFetch,
                request(HttpMethod::Post,
                        std::string_view(url),
                        testing::Eq(testing::ByRef(json))))
      .Times(1)
      .WillOnce(testing::Return(ok(makeEmptyResponse())));
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.roomIdPost(id, name, "connect");
    EXPECT_TRUE(result) << result.err();
  }
  /**
   * @brief
   * roomIdPostにおいてDisonnectリクエストを出し、Fetchが成功した時にApiBridgeが何も返さないかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, roomIdPostOkDisonnect) {
    test::MockFetch mockFetch;
    std::uint64_t id = 7040782538;
    std::string name
      = "soon's macbook air 13manyenguraidegakuwaridekaemashitaureshiine!";
    auto url  = "/room/" + std::to_string(id);
    auto json = makeJson(
      R"({"name": "soon's macbook air 13manyenguraidegakuwaridekaemashitaureshiine!", "request": "disconnect"})");
    EXPECT_CALL(mockFetch,
                request(HttpMethod::Post,
                        std::string_view(url),
                        testing::Eq(testing::ByRef(json))))
      .Times(1)
      .WillOnce(testing::Return(ok(makeEmptyResponse())));
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.roomIdPost(id, name, "disconnect");
    EXPECT_TRUE(result) << result.err();
  }
  /**
   * @brief
   * roomIdPostにおいてFetchがcURLの接続に失敗した時にApiBridgeがエラーを返してくれるかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, roomIdPostErrCurl) {
    test::MockFetch mockFetch;
    std::uint64_t id = 7040782538;
    std::string name
      = "soon's macbook air 13manyenguraidegakuwaridekaemashitaureshiine!";
    auto url  = "/room/" + std::to_string(id);
    auto json = makeJson(
      R"({"name": "soon's macbook air 13manyenguraidegakuwaridekaemashitaureshiine!", "request": "connect"})");
    EXPECT_CALL(mockFetch,
                request(HttpMethod::Post,
                        std::string_view(url),
                        testing::Eq(testing::ByRef(json))))
      .Times(1)
      .WillOnce(testing::Return(makeError(ERR_CURL_CONNECTION_FAILED, "")));
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.roomIdPost(id, name, "connect");
    EXPECT_FALSE(result);
    EXPECT_EQ(result.err().code, ERR_CURL_CONNECTION_FAILED) << result.err();
  }
  /**
   * @brief
   * roomIdPostにおいてFetchが2xx以外のステータスコードを返すときにサーバからもらうエラーレスポンスをそのまま返してくれるかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, roomIdPost2xx) {
    test::MockFetch mockFetch;
    std::uint64_t id = 7040782538;
    std::string name
      = "soon's macbook air 13manyenguraidegakuwaridekaemashitaureshiine!";
    auto url  = "/room/" + std::to_string(id);
    auto json = makeJson(
      R"({"name": "soon's macbook air 13manyenguraidegakuwaridekaemashitaureshiine!", "request": "connect"})");
    EXPECT_CALL(mockFetch,
                request(HttpMethod::Post,
                        std::string_view(url),
                        testing::Eq(testing::ByRef(json))))
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
    auto result = apiBridge.roomIdPost(id, name, "connect");
    EXPECT_FALSE(result);
    EXPECT_EQ(result.err().code, "ERR_BAD_REQUEST") << result.err();
  }
  /**
   * @brief
   * roomIdContentGetにおいてFetchが成功し、バイナリが送られてきた時にApiBridgeがバイナリを返してくれるかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, roomIdContentGetOk) {
    test::MockFetch mockFetch;
    std::uint64_t id = 7040782538;
    auto url         = "/room/" + std::to_string(id) + "/content";

    EXPECT_CALL(mockFetch, request(HttpMethod::Get, std::string_view(url)))
      .Times(1)
      .WillOnce(testing::Return(ok(makeBinaryResponse())));
    std::string data = "AAABBBCCC";
    std::vector<uint8_t> body;
    body.resize(data.size());
    std::copy(data.begin(), data.end(), body.begin());
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.roomIdContentGet(id);
    EXPECT_TRUE(result) << result.err();
    EXPECT_EQ(result.get(), body) << toString(result.get());
  }
  /**
   * @brief
   * roomIdContentGetにおいてFetchが成功し、jsonが送られてきた時にApiBridgeがエラーを返してくれるかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, roomIdContentGetErrJsonInvalid) {
    test::MockFetch mockFetch;
    std::uint64_t id = 7040782538;
    auto url         = "/room/" + std::to_string(id) + "/content";

    EXPECT_CALL(mockFetch, request(HttpMethod::Get, std::string_view(url)))
      .Times(1)
      .WillOnce(testing::Return(ok(makeJsonResponse(
        R"({"name": "soon's macbook air 13manyenguraidegakuwaridekaemashitaureshiine!"})"))));
    std::string data = "AAABBBCCC";
    std::vector<uint8_t> body;
    body.resize(data.size());
    std::copy(data.begin(), data.end(), body.begin());
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.roomIdContentGet(id);
    EXPECT_FALSE(result) << toString(result.get());
    EXPECT_EQ(
      result.err(),
      (ErrorResponse{ .code   = ERR_INVALID_RESPONSE,
                      .reason = "Invalid response, binary not returned" }))
      << result.err();
  }
  /**
   * @brief
   * roomIdContentGetにおいてFetchが2xx以外のステータスコードを返すときにサーバからもらうエラーレスポンスをそのまま返してくれるかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, roomIdContentGet2xx) {
    test::MockFetch mockFetch;
    std::uint64_t id = 7040782538;
    auto url         = "/room/" + std::to_string(id) + "/content";
    EXPECT_CALL(mockFetch, request(HttpMethod::Get, std::string_view(url)))
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
    auto result = apiBridge.roomIdContentGet(id);
    EXPECT_FALSE(result) << toString(result.get());
    EXPECT_EQ(result.err().code, "ERR_BAD_REQUEST") << result.err();
  }
  /**
   * @brief
   * roomIdContentDeleteにおいてFetchが成功し、ApiBridge何も返さないかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, roomIdContentDeleteOk) {
    test::MockFetch mockFetch;
    std::uint64_t id = 7040782538;
    auto url         = "/room/" + std::to_string(id) + "/content";
    EXPECT_CALL(mockFetch, request(HttpMethod::Delete, std::string_view(url)))
      .Times(1)
      .WillOnce(testing::Return(ok(makeEmptyResponse())));
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.roomIdContentDelete(id);
    EXPECT_TRUE(result) << result.err();
  }

  /**
   * @brief
   * roomIdContentDeleteにおいてFetchが2xx以外のステータスコードを返すときにサーバからもらうエラーレスポンスをそのまま返してくれるかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, roomIdContentDelete2xx) {
    test::MockFetch mockFetch;
    std::uint64_t id = 7040782538;
    auto url         = "/room/" + std::to_string(id) + "/content";
    EXPECT_CALL(mockFetch, request(HttpMethod::Delete, std::string_view(url)))
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
    auto result = apiBridge.roomIdContentDelete(id);
    EXPECT_FALSE(result);
    EXPECT_EQ(result.err().code, "ERR_BAD_REQUEST") << result.err();
  }
  /**
   * @brief
   * roomIdContentPutにおいてバイナリを渡したときにFetchが成功し、ApiBridge何も返さないかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, roomIdContentPutOkBin) {
    test::MockFetch mockFetch;
    std::uint64_t id = 7040782538;
    std::string data = "AAABBBCCC";
    std::vector<uint8_t> body;
    body.resize(data.size());
    std::copy(data.begin(), data.end(), body.begin());
    std::string mime = "application/pdf";
    auto url         = "/room/" + std::to_string(id) + "/content";
    EXPECT_CALL(
      mockFetch,
      request(
        HttpMethod::Put, std::string_view(url), std::string_view(mime), body))
      .Times(1)
      .WillOnce(testing::Return(ok(makeEmptyResponse())));
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.roomIdContentPut(id, body, mime);
    EXPECT_TRUE(result);
  }
  /**
   * @brief
   * roomIdContentPutにおいて文字列を渡したときにFetchが成功し、ApiBridge何も返さないかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, roomIdContentPutOkStr) {
    test::MockFetch mockFetch;
    std::uint64_t id = 7040782538;
    std::string data = "AAABBBCCC";
    std::string mime = "text/nanika";
    std::vector<uint8_t> body;
    body.resize(data.size());
    std::copy(data.begin(), data.end(), body.begin());
    auto url = "/room/" + std::to_string(id) + "/content";
    EXPECT_CALL(
      mockFetch,
      request(
        HttpMethod::Put, std::string_view(url), std::string_view(mime), body))
      .Times(1)
      .WillOnce(testing::Return(ok(makeEmptyResponse())));
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.roomIdContentPut(id, toBinary(data), mime);
    EXPECT_TRUE(result);
  }
  /**
   * @brief
   * roomIdContentPutにおいてFetchが2xx以外のステータスコードを返すときにサーバからもらうエラーレスポンスをそのまま返してくれるかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, roomIdContentPut2xx) {
    test::MockFetch mockFetch;
    std::uint64_t id = 7040782538;
    std::string data = "AAABBBCCC";
    std::vector<uint8_t> body;
    body.resize(data.size());
    std::copy(data.begin(), data.end(), body.begin());
    std::string mime = "application/pdf";
    auto url         = "/room/" + std::to_string(id) + "/content";
    EXPECT_CALL(
      mockFetch,
      request(
        HttpMethod::Put, std::string_view(url), std::string_view(mime), body))
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
    auto result = apiBridge.roomIdContentPut(id, body, mime);
    EXPECT_FALSE(result);
    EXPECT_EQ(result.err().code, "ERR_BAD_REQUEST") << result.err();
  }
  /**
   * @brief
   * roomIdStatusGetにおいてFetchが成功し、ファイル形式で帰ってきた時に、ApiBridgeがルームのステータスを返してくれるかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, roomIdStatusGetOkFile) {
    test::MockFetch mockFetch;
    std::uint64_t id = 7040782538;
    auto url         = "/room/" + std::to_string(id) + "/status";
    EXPECT_CALL(mockFetch, request(HttpMethod::Get, std::string_view(url)))
      .Times(1)
      .WillOnce(testing::Return(ok(makeJsonResponse(R"(
        {
          "device": "soon's windows",
          "timestamp": 20202020,
          "type": "file",
          "mime": "application/pdf",
          "name": "filename",
          "hash": "101010"
          }
        )"))));
    ContentStatus contentStatus{
      .device    = "soon's windows",
      .timestamp = 20202020,
      .type      = ContentType::File,
      .name      = "filename",
      .mime      = "application/pdf",
    };
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.roomIdStatusGet(id);
    EXPECT_TRUE(result) << result.err();
    EXPECT_EQ(result.get().first, contentStatus)
      << result.get().first << contentStatus;
    EXPECT_EQ(result.get().second, "101010") << result.get().second;
  }
  /**
   * @brief
   * roomIdStatusGetにおいてFetchが成功し、クリップボード形式で帰ってきた時に、ApiBridgeがルームのステータスを返してくれるかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, roomIdStatusGetOkClip) {
    test::MockFetch mockFetch;
    std::uint64_t id = 7040782538;
    auto url         = "/room/" + std::to_string(id) + "/status";
    EXPECT_CALL(mockFetch, request(HttpMethod::Get, std::string_view(url)))
      .Times(1)
      .WillOnce(testing::Return(ok(makeJsonResponse(R"(
        {
          "device": "soon's windows",
          "timestamp": 20202020,
          "type": "clipboard",
          "mime": "text",
          "name": "",
          "hash": "101010"
          }
        )"))));
    ContentStatus contentStatus{
      .device    = "soon's windows",
      .timestamp = 20202020,
      .type      = ContentType::Clipboard,
      .name      = "",
      .mime      = "text",
    };
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.roomIdStatusGet(id);
    EXPECT_TRUE(result) << result.err();
    EXPECT_EQ(result.get().first, contentStatus)
      << result.get().first << contentStatus;
    EXPECT_EQ(result.get().second, "101010") << result.get().second;
  }
  /**
   * @brief
   * roomIdStatusGetにおいてFetchが成功し、クリップボード形式で帰ってきた時に、なぜかファイル名があったとしても、ApiBridgeがname
   * = "" であるようなルームのステータスを返してくれるかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, roomIdStatusGetOkClipName) {
    test::MockFetch mockFetch;
    std::uint64_t id = 7040782538;
    auto url         = "/room/" + std::to_string(id) + "/status";
    EXPECT_CALL(mockFetch, request(HttpMethod::Get, std::string_view(url)))
      .Times(1)
      .WillOnce(testing::Return(ok(makeJsonResponse(R"(
        {
          "device": "soon's windows",
          "timestamp": 20202020,
          "type": "clipboard",
          "mime": "text",
          "name": "nazekanamaegaaru",
          "hash": "101010"
          }
        )"))));
    ContentStatus contentStatus{
      .device    = "soon's windows",
      .timestamp = 20202020,
      .type      = ContentType::Clipboard,
      .name      = "",
      .mime      = "text",
    };

    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.roomIdStatusGet(id);
    EXPECT_TRUE(result) << result.err();
    EXPECT_EQ(result.get().first, contentStatus)
      << result.get().first << contentStatus;
    EXPECT_EQ(result.get().second, "101010") << result.get().second;
  }
  /**
   * @brief
   * roomIdStatusGetにおいてFetchが成功したがレスポンスの誤りがあったときに、ApiBridgeがエラーを返してくれるかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, roomIdStatusGetErrResponse) {
    test::MockFetch mockFetch;
    std::uint64_t id = 7040782538;
    auto url         = "/room/" + std::to_string(id) + "/status";
    EXPECT_CALL(mockFetch, request(HttpMethod::Get, std::string_view(url)))
      .Times(1)
      .WillOnce(testing::Return(ok(makeJsonResponse(R"(
        {
          "device": "soon's windows",
          "timestamp": 20202020,
          "type": "clipboard",
          "mime": "application/pdf",
          "name": ""
          }
        )"))));
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.roomIdStatusGet(id);
    EXPECT_FALSE(result) << result.get().first << " " << result.get().second;
    EXPECT_EQ(result.err().code, ERR_INVALID_RESPONSE) << result.err().code;
  }
  /**
   * @brief
   * roomIdStatusGetにおいてFetchが2xx以外のステータスコードを返すときにサーバからもらうエラーレスポンスをそのまま返してくれるかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, roomIdStatusGet2xx) {
    test::MockFetch mockFetch;
    std::uint64_t id = 7040782538;
    auto url         = "/room/" + std::to_string(id) + "/status";
    EXPECT_CALL(mockFetch, request(HttpMethod::Get, std::string_view(url)))
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
    auto result = apiBridge.roomIdStatusGet(id);
    EXPECT_FALSE(result) << result.get().first << " " << result.get().second;
    EXPECT_EQ(result.err().code, "ERR_BAD_REQUEST") << result.err();
  }
  /**
   * @brief
   * roomIdStatusDeleteにおいてFetchが成功したときにApiBridgeが何も返さないかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, roomIdStatusDeleteOk) {
    test::MockFetch mockFetch;
    std::uint64_t id = 7040782538;
    auto url         = "/room/" + std::to_string(id) + "/status";
    EXPECT_CALL(mockFetch, request(HttpMethod::Delete, std::string_view(url)))
      .Times(1)
      .WillOnce(testing::Return(ok(makeEmptyResponse())));
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.roomIdStatusDelete(id);
    EXPECT_TRUE(result) << result.err();
  }
  /**
   * @brief
   * roomIdStatusDeleteにおいてFetchが2xx以外のステータスコードを返すときにサーバからもらうエラーレスポンスをそのまま返してくれるかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, roomIdStatusDelete2xx) {
    test::MockFetch mockFetch;
    std::uint64_t id = 7040782538;
    auto url         = "/room/" + std::to_string(id) + "/status";
    EXPECT_CALL(mockFetch, request(HttpMethod::Delete, std::string_view(url)))
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
    auto result = apiBridge.roomIdStatusDelete(id);
    EXPECT_FALSE(result);
    EXPECT_EQ(result.err().code, "ERR_BAD_REQUEST") << result.err();
  }
  /**
   * @brief
   * roomIdStatusPutにおいてFileを送信してFetchが成功したときにApiBridgeが何も返さないかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, roomIdStatusPutOkFile) {
    test::MockFetch mockFetch;
    std::uint64_t id = 7040782538;
    std::string hash = "20202020";
    ContentStatus contentStatus{};
    contentStatus.device    = "soon's macbook";
    contentStatus.mime      = "application/pdf";
    contentStatus.name      = "";
    contentStatus.timestamp = 20202020;
    contentStatus.type      = ContentType::File;
    auto json               = makeJson(R"({
      "device": "soon's macbook",
      "mime": "application/pdf",
      "name": "",
      "timestamp": 20202020,
      "type": "file",
      "hash": "20202020"
    })");
    auto url                = "/room/" + std::to_string(id) + "/status";
    EXPECT_CALL(mockFetch,
                request(HttpMethod::Put,
                        std::string_view(url),
                        testing::Eq(testing::ByRef(json))))
      .Times(1)
      .WillOnce(testing::Return(ok(makeEmptyResponse())));
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.roomIdStatusPut(id, contentStatus, hash);
    EXPECT_TRUE(result) << result.err();
  }
  /**
   * @brief
   * roomIdStatusPutにおいてClipboardを送信してFetchが成功したときにApiBridgeが何も返さないかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, roomIdStatusPutOkClip) {
    test::MockFetch mockFetch;
    std::uint64_t id = 7040782538;
    std::string hash = "20202020";
    ContentStatus contentStatus{};
    contentStatus.device    = "soon's macbook";
    contentStatus.mime      = "text/plain";
    contentStatus.name      = "";
    contentStatus.timestamp = 20202020;
    contentStatus.type      = ContentType::Clipboard;
    auto json               = makeJson(R"({
      "device": "soon's macbook",
      "mime": "text/plain",
      "name": "",
      "timestamp": 20202020,
      "type": "clipboard",
      "hash": "20202020"
    })");
    auto url                = "/room/" + std::to_string(id) + "/status";
    EXPECT_CALL(mockFetch,
                request(HttpMethod::Put,
                        std::string_view(url),
                        testing::Eq(testing::ByRef(json))))
      .Times(1)
      .WillOnce(testing::Return(ok(makeEmptyResponse())));
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.roomIdStatusPut(id, contentStatus, hash);
    EXPECT_TRUE(result) << result.err();
  }
  /**
   * @brief
   * roomIdStatusPutにおいてClipboardを送信してmimeにtext/plainとは異なる値が入っていたら、
   * roomIdStatusPut内部でmimeの中身をtext/plainに書き換え、それから
   * ApiBridgeが何も返さないかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, roomIdStatusPutOkClipMimeCollapse) {
    test::MockFetch mockFetch;
    std::uint64_t id = 7040782538;
    std::string hash = "20202020";
    ContentStatus contentStatus{};
    contentStatus.device    = "soon's macbook";
    contentStatus.mime      = "image/png";
    contentStatus.name      = "";
    contentStatus.timestamp = 20202020;
    contentStatus.type      = ContentType::Clipboard;
    auto json               = makeJson(R"({
      "device": "soon's macbook",
      "mime": "text/plain",
      "name": "",
      "timestamp": 20202020,
      "type": "clipboard",
      "hash": "20202020"
    })");
    auto url                = "/room/" + std::to_string(id) + "/status";
    EXPECT_CALL(mockFetch,
                request(HttpMethod::Put,
                        std::string_view(url),
                        testing::Eq(testing::ByRef(json))))
      .Times(1)
      .WillOnce(testing::Return(ok(makeEmptyResponse())));
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.roomIdStatusPut(id, contentStatus, hash);
    EXPECT_TRUE(result) << result.err();
  }
  /**
   * @brief
   * roomIdStatusPutにおいてMultiFileを送信してFetchが成功したときにApiBridgeが何も返さないかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, roomIdStatusPutOkMulti) {
    test::MockFetch mockFetch;
    std::uint64_t id = 7040782538;
    std::string hash = "20202020";
    ContentStatus contentStatus{};
    contentStatus.device    = "soon's macbook";
    contentStatus.mime      = "application/x-7z-compressed";
    contentStatus.name      = "";
    contentStatus.timestamp = 20202020;
    contentStatus.type      = ContentType::MultiFile;
    auto json               = makeJson(R"({
      "device": "soon's macbook",
      "mime": "application/x-7z-compressed",
      "name": "",
      "timestamp": 20202020,
      "type": "multi-file",
      "hash": "20202020"
    })");
    auto url                = "/room/" + std::to_string(id) + "/status";
    EXPECT_CALL(mockFetch,
                request(HttpMethod::Put,
                        std::string_view(url),
                        testing::Eq(testing::ByRef(json))))
      .Times(1)
      .WillOnce(testing::Return(ok(makeEmptyResponse())));
    ApiBridge apiBridge(&mockFetch);
    auto result = apiBridge.roomIdStatusPut(id, contentStatus, hash);
    EXPECT_TRUE(result) << result.err();
  }
  /**
   * @brief
   * roomIdStatusPutにおいてFetchが2xx以外のステータスコードを返すときにサーバからもらうエラーレスポンスをそのまま返してくれるかどうかをテストする。
   *
   */
  TEST(ApiBridgeTest, roomIdStatusPut2xx) {
    test::MockFetch mockFetch;
    std::uint64_t id = 7040782538;
    std::string hash = "20202020";
    ContentStatus contentStatus{};
    contentStatus.device    = "soon's macbook";
    contentStatus.mime      = "application/x-7z-compressed";
    contentStatus.name      = "";
    contentStatus.timestamp = 20202020;
    contentStatus.type      = ContentType::MultiFile;
    auto json               = makeJson(R"({
      "device": "soon's macbook",
      "mime": "application/x-7z-compressed",
      "name": "",
      "timestamp": 20202020,
      "type": "multi-file",
      "hash": "20202020"
    })");
    auto url                = "/room/" + std::to_string(id) + "/status";
    EXPECT_CALL(mockFetch,
                request(HttpMethod::Put,
                        std::string_view(url),
                        testing::Eq(testing::ByRef(json))))
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
    auto result = apiBridge.roomIdStatusPut(id, contentStatus, hash);
    EXPECT_FALSE(result);
    EXPECT_EQ(result.err().code, "ERR_BAD_REQUEST") << result.err();
  }
} // namespace octane::internal
