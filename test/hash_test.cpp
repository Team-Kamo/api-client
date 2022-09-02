#include "include/internal/hash.h"

#include <gtest/gtest.h>

#include "include/internal/http_client.h"

namespace octane::internal {
  TEST(HashTest, ConvToHex) {
    // 10未満でも二桁になるかテスト
    EXPECT_EQ(convToHex({ 0 }), "00");
    EXPECT_EQ(convToHex({ 9 }), "09");
    // a-fがちゃんと出るかテスト
    EXPECT_EQ(convToHex({ 10 }), "0a");
    EXPECT_EQ(convToHex({ 171 }), "ab");
    // 複数の要素があってもちゃんとできるかテスト
    EXPECT_EQ(convToHex({ 0, 8, 10, 16, 205, 255 }), "00080a10cdff");
  }

  /**
   * 検証用に次のウェブサービスでハッシュを生成しています。
   * https://www.toolkitbay.com/tkb/tool/BLAKE2b_256
   */

  TEST(HashTest, GenerateHashA) {
    std::string message = "Impossible is nothing.";
    std::string digest
      = "a61ad9c914a0a68c50c5f87537ae152c6d233ebb79ad321ad3e89787d7279aa2";

    std::vector<std::uint8_t> src;
    src.resize(message.size());
    std::copy(message.begin(), message.end(), src.begin());
    EXPECT_EQ(generateHash(src), digest);
  }
  TEST(HashTest, GenerateHashB) {
    std::string message = "Do your best.";
    std::string digest
      = "e5f82d8e538c516f946db5f8fad590412458a1fec09bac56630695d0558b0c60";

    std::vector<std::uint8_t> src;
    src.resize(message.size());
    std::copy(message.begin(), message.end(), src.begin());
    EXPECT_EQ(generateHash(src), digest);
  }
  TEST(HashTest, GenerateHashC) {
    std::string message
      = "God doesn't require us to succeed; he only requires that you try.";
    std::string digest
      = "b358a76b01dc65389326e2e2d66c2cb0118b874bfe7e8d2dbac8889160e6d60b";

    std::vector<std::uint8_t> src;
    src.resize(message.size());
    std::copy(message.begin(), message.end(), src.begin());
    EXPECT_EQ(generateHash(src), digest);
  }
} // namespace octane::internal