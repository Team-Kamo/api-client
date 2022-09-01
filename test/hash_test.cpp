#include "include/internal/hash.h"

#include <gtest/gtest.h>

#include <sstream>
#include <string>

#include "include/internal/http_client.h"

namespace octane::internal {

  // https://www.toolkitbay.com/tkb/tool/BLAKE2b_256

  std::vector<std::uint8_t> makeBinary(std::string_view msg) {
    std::vector<std::uint8_t> bin;
    while (!msg.empty()) {
      std::stringstream ss;
      ss << std::hex << msg.substr(0, 2);
      int n;
      ss >> n;
      bin.push_back(n);
      msg = msg.substr(2);
    }
    return bin;
  }
  TEST(HashTest, GenerateHashA) {
    std::string message = "Impossible is nothing.";
    std::string digest
      = "a61ad9c914a0a68c50c5f87537ae152c6d233ebb79ad321ad3e89787d7279aa2";

    std::vector<std::uint8_t> src;
    src.resize(message.size());
    std::copy(message.begin(), message.end(), src.begin());
    EXPECT_EQ(generateHash(src), makeBinary(digest));
  }
  TEST(HashTest, GenerateHashB) {
    std::string message = "Do your best.";
    std::string digest
      = "e5f82d8e538c516f946db5f8fad590412458a1fec09bac56630695d0558b0c60";

    std::vector<std::uint8_t> src;
    src.resize(message.size());
    std::copy(message.begin(), message.end(), src.begin());
    EXPECT_EQ(generateHash(src), makeBinary(digest));
  }
  TEST(HashTest, GenerateHashC) {
    std::string message
      = "God doesn't require us to succeed; he only requires that you try.";
    std::string digest
      = "b358a76b01dc65389326e2e2d66c2cb0118b874bfe7e8d2dbac8889160e6d60b";

    std::vector<std::uint8_t> src;
    src.resize(message.size());
    std::copy(message.begin(), message.end(), src.begin());
    EXPECT_EQ(generateHash(src), makeBinary(digest));
  }
} // namespace octane::internal