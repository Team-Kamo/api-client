#include "include/internal/multi_file.h"

#include <gtest/gtest.h>

#include <iostream>

namespace octane::internal {
  namespace {
    std::vector<std::uint8_t> toBinary(std::string_view str) {
      std::vector<std::uint8_t> vec;
      vec.reserve(str.size());
      std::copy(str.begin(), str.end(), std::back_inserter(vec));
      return vec;
    }
  } // namespace
  TEST(MultiFileCompressorTest, MultiFile) {
    auto data = MultiFileCompressor::compress({
      FileInfo{
        .filename = "hello.txt",
        .data     = toBinary("Hello World"),
      },
      FileInfo{
        .filename = "wawawa.txt",
        .data     = toBinary("hohohoho"),
      },
      FileInfo{
        .filename = "aaa/bbb.txt",
        .data     = toBinary("yahoo!"),
      },
    });
    ASSERT_TRUE(data) << data.err();
    auto files = MultiFileDecompressor::decompress(data.get());
    ASSERT_TRUE(files) << files.err();
    ASSERT_EQ(files.get().size(), 3);
    EXPECT_EQ(files.get()[0].filename, "hello.txt");
    EXPECT_EQ(files.get()[0].data, toBinary("Hello World"));
  }
} // namespace octane::internal