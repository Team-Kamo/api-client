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
#include <string>

namespace octane::internal {
  /**
   * @brief バイナリシーケンスを16進数文字列に変換する。
   * @details
   * 標準ライブラリにそんくらいの機能あるやろとか言ってはいけない。
   * 
   * @param[in] data 変換元のデータ
   * @return std::string 変換したデータ
   */
  std::string convToHex(const std::vector<std::uint8_t>& data);
  /**
   * @brief ハッシュ値を生成する。
   * @details
   * 使用されるアルゴリズムはblack2b-256である。
   *
   * @param[in] src ハッシュを生成する値。
   * @return std::string 生成されたハッシュ値。
   */
  std::string generateHash(const std::vector<std::uint8_t>& src);
} // namespace octane::internal

#endif // OCTANE_API_CLIENT_INTERNAL_HASH_H_