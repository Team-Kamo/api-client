/**
 * @file hash.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief ハッシュアルゴリズムを提供する。
 * @version 0.1
 * @date 2022-09-02
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef OCTANE_API_CLIENT_INTERNAL_HASH_H_
#define OCTANE_API_CLIENT_INTERNAL_HASH_H_

#include <vector>

namespace octane::internal {
  /**
   * @brief ハッシュ値を生成する。
   * @details
   * 使用されるアルゴリズムはblack2b-256である。
   *
   * @param[in] src ハッシュを生成する値。
   * @return std::vector<std::uint8_t> 生成されたハッシュ値。
   */
  std::vector<std::uint8_t> generateHash(const std::vector<std::uint8_t>& src);
} // namespace octane::internal

#endif // OCTANE_API_CLIENT_INTERNAL_HASH_H_