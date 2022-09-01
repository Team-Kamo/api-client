#include "include/internal/http_client.h"

#include <gtest/gtest.h>

namespace octane::internal {

  TEST(HttpClientTest, WriteCallback) {
    HttpClient client;
    client.init();

    constexpr size_t length = 10;
    char* buffer            = (char*)"ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    std::vector<std::uint8_t> chunk;
    ASSERT_EQ(client.writeCallback(buffer, 1, length, &chunk), length);
    for (size_t i = 0; i < length; ++i) {
      ASSERT_EQ(buffer[i], chunk[i]);
    }

    ASSERT_EQ(client.writeCallback(buffer + length, 1, length, &chunk), length);
    for (size_t i = 0; i < length; ++i) {
      ASSERT_EQ(buffer[length + i], chunk[length + i]);
    }
  }
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
  TEST(HttpClientTest, HeaderCallback) {
    constexpr const char* const headers[] = {
      "Allow: GET,POST,PUT,DELETE",
      "Content-Type: application/json; charset=utf-8",
      "Content-Length: 500",
    };

    HttpClient client;
    client.init();

    std::map<std::string, std::string> responseHeaderField;

    for (auto header : headers) {
      client.headerCallback(
        (char*)header, 1, strlen(header), &responseHeaderField);
    }

    ASSERT_TRUE(responseHeaderField.contains("Allow"));
    ASSERT_TRUE(responseHeaderField.contains("Content-Type"));
    ASSERT_TRUE(responseHeaderField.contains("Content-Length"));

    ASSERT_EQ(responseHeaderField["Allow"], "GET,POST,PUT,DELETE");
    ASSERT_EQ(responseHeaderField["Content-Type"],
              "application/json; charset=utf-8");
    ASSERT_EQ(responseHeaderField["Content-Length"], "500");
  }

} // namespace octane::internal