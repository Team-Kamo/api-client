#include "include/internal/hash.h"

#include <blake2.h>
#include <cryptlib.h>

namespace octane::internal {
  std::vector<std::uint8_t> generateHash(const std::vector<std::uint8_t>& src) {
    std::vector<std::uint8_t> digest;

    CryptoPP::BLAKE2b blake2(32u);
    blake2.Update(src.data(), src.size());
    digest.resize(blake2.DigestSize());
    blake2.Final(digest.data());

    return digest;
  }
} // namespace octane::internal