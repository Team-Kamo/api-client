#include "include/internal/http_client.h"

#include <gtest/gtest.h>

#include "include/error_code.h"

namespace octane::internal {
  /**
   * @brief
   * GETリクエストでボディを指定したときにちゃんとエラーとなるかテストする。
   *
   */
  TEST(HttpClientTest, ExpectAnErrorWhenABodyPartExistsInGetRequest) {
    HttpClient client;
    client.init();

    std::vector<std::uint8_t> body = { 'G', 'E', 'T' };
    HttpRequest request{
      .method      = HttpMethod::Get,
      .version     = HttpVersion::Http2,
      .uri         = "/api/v1/health",
      .headerField = {},
      .body        = &body,
    };
    auto response = client.request("http://localhost:3000", request);
    EXPECT_FALSE(response);

    EXPECT_EQ(response.err().code, ERR_INCORRECT_HTTP_METHOD);
  }
  /**
   * @brief
   * DELETEリクエストでボディを指定したときにちゃんとエラーとなるかテストする。
   *
   */
  TEST(HttpClientTest, ExpectAnErrorWhenABodyPartExistsInDeleteRequest) {
    HttpClient client;
    client.init();

    std::vector<std::uint8_t> body = { 'D', 'E', 'L', 'E', 'T', 'E' };
    HttpRequest request{
      .method      = HttpMethod::Delete,
      .version     = HttpVersion::Http2,
      .uri         = "/api/v1/health",
      .headerField = {},
      .body        = &body,
    };
    auto response = client.request("http://localhost:3000", request);
    EXPECT_FALSE(response);

    EXPECT_EQ(response.err().code, ERR_INCORRECT_HTTP_METHOD);
  }
  /**
   * @brief HttpClient::writeCallbackが正常に動作するかをテストする。
   *
   */
  TEST(HttpClientTest, WriteCallback) {
    HttpClient client;
    client.init();

    constexpr size_t length = 10;
    char* buffer            = (char*)"ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    HttpResponse response;
    ASSERT_EQ(client.writeCallback(buffer, 1, length, &response), length);
    for (size_t i = 0; i < length; ++i) {
      ASSERT_EQ(buffer[i], response.body[i]);
    }

    ASSERT_EQ(client.writeCallback(buffer + length, 1, length, &response),
              length);
    for (size_t i = 0; i < length; ++i) {
      ASSERT_EQ(buffer[length + i], response.body[length + i]);
    }
  }
  /**
   * @brief HttpClient::readCallbackが正常に動作するかをテストする。
   *
   */
  TEST(HttpClientTest, ReadCallback) {
    HttpClient client;
    client.init();

    constexpr const char str[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::vector<std::uint8_t> body;
    for (auto c : str) body.push_back(c);

    std::pair<const std::vector<std::uint8_t>*, size_t> stream{ &body, 0 };

    std::string buffer;
    buffer.resize(30);

    ASSERT_EQ(client.readCallback(buffer.data(), 1, 10, &stream), 10);
    for (size_t i = 0; i < 10; ++i) {
      ASSERT_EQ(buffer[i], body[i]);
    }

    ASSERT_EQ(client.readCallback(buffer.data() + 10, 1, 15, &stream), 15);
    for (size_t i = 0; i < 15; ++i) {
      ASSERT_EQ(buffer[i + 10], body[i + 10]);
    }
  }
  /**
   * @brief HttpClient::headerCallbackが正常に動作するかをテストする。
   *
   */
  TEST(HttpClientTest, HeaderCallback) {
    constexpr const char* const headers[] = {
      "HTTP/2 200 OK\r\n",
      "Allow: GET,POST,PUT,DELETE\r\n",
      "Content-Type: text/html; charset=utf-8\r\n",
      "Content-Length: 500\r\n",
      "\r\n",
    };

    HttpClient client;
    client.init();

    HttpResponse response;

    for (auto header : headers) {
      client.headerCallback((char*)header, 1, strlen(header), &response);
    }

    auto& statusLine          = response.statusLine;
    auto& responseHeaderField = response.headerField;

    EXPECT_TRUE(responseHeaderField.contains("Allow"));
    EXPECT_TRUE(responseHeaderField.contains("Content-Type"));
    EXPECT_TRUE(responseHeaderField.contains("Content-Length"));

    EXPECT_EQ(statusLine, "HTTP/2 200 OK");

    EXPECT_EQ(responseHeaderField["Allow"], "GET,POST,PUT,DELETE");
    EXPECT_EQ(responseHeaderField["Content-Type"], "text/html; charset=utf-8");
    EXPECT_EQ(responseHeaderField["Content-Length"], "500");
  }
  /**
   * @brief HttpClient::makeHttpResponseが正常に動作するかをテストする。
   *
   */
  TEST(HttpClientTest, MakeHttpResponseOk) {
    HttpClient client;
    client.init();

    HttpResponse response1;
    response1.statusLine                    = "HTTP/2 200 OK";
    response1.headerField["Allow"]          = "GET,POST,PUT,DELETE";
    response1.headerField["Content-Type"]   = "text/html; charset=utf-8";
    response1.headerField["Content-Length"] = "500";
    constexpr const char str[]              = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for (auto c : str) response1.body.push_back(c);

    HttpResponse response2{};
    response2.body                          = response1.body;
    response2.headerField["Allow"]          = "GET,POST,PUT,DELETE";
    response2.headerField["Content-Type"]   = "text/html; charset=utf-8";
    response2.headerField["Content-Length"] = "500";
    response2.statusCode                    = 200;
    response2.statusLine                    = "HTTP/2 200 OK";
    response2.version                       = HttpVersion::Http2;

    auto result = client.makeHttpResponse(std::move(response1));
    EXPECT_TRUE(result) << response2 << result.err();
    EXPECT_EQ(result.get(), response2) << response2 << result.get();
  }
  /**
   * @brief
   * HttpClient::makeHttpResponseにおいて返されたヘッダのHTTPのバージョンにエラーがあった場合、エラーを返すかどうかをテストする。
   *
   */
  TEST(HttpClientTest, MakeHttpResponseErr) {
    HttpClient client;
    client.init();

    HttpResponse response;
    response.statusLine                    = "HTTP/334 200 OK";
    response.headerField["Allow"]          = "GET,POST,PUT,DELETE";
    response.headerField["Content-Type"]   = "text/html; charset=utf-8";
    response.headerField["Content-Length"] = "500";
    constexpr const char str[]             = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for (auto c : str) response.body.push_back(c);

    auto result = client.makeHttpResponse(std::move(response));
    EXPECT_FALSE(result) << result.get();
    EXPECT_EQ(result.err().code, ERR_INVALID_RESPONSE) << result.err().code;
  }
} // namespace octane::internal