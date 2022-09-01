#ifndef OCTANE_API_CLIENT_INTERNAL_HASH_H_
#define OCTANE_API_CLIENT_INTERNAL_HASH_H_

#include <vector>

namespace octane::internal {
  std::vector<std::uint8_t> generateHash(const std::vector<std::uint8_t>& src);
} // namespace octane::internal

#endif // OCTANE_API_CLIENT_INTERNAL_HASH_H_