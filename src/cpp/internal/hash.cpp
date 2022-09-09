/**
 * @file hash.cpp
 * @author soon (kento.soon@gmail.com)
 * @brief hash.hの実装
 * @version 0.1
 * @date 2022-09-05
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "include/internal/hash.h"

#include <blake2.h>
#include <cryptlib.h>

namespace octane::internal {
  char convToHexMono(std::uint8_t number) {
    if (number < 10) {
      return '0' +  number;
    } else {
      return 'a' + number - 10;
    }
  }
  std::string convToHex(const std::vector<std::uint8_t>& data) {
    std::string ans;
    for (auto number : data) {
      std::uint8_t left = number >> 4;
      std::uint8_t right = number & 0x0f;
      ans.push_back(convToHexMono(left));
      ans.push_back(convToHexMono(right));
    }
    return ans;
  }
  std::string generateHash(const std::vector<std::uint8_t>& src) {
    std::vector<std::uint8_t> digest;

    CryptoPP::BLAKE2b blake2(32u);
    blake2.Update(src.data(), src.size());
    digest.resize(blake2.DigestSize());
    blake2.Final(digest.data());

    return convToHex(digest);
  }
} // namespace octane::internal