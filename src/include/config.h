/**
 * @file config.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief コンパイル時に使用する定数を定義する。
 * @version 0.1
 * @date 2022-09-02
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef OCTANE_API_CLIENT_CONFIG_H_
#define OCTANE_API_CLIENT_CONFIG_H_

namespace octane {
  /** ライブラリ名。Octane API Client。*/
  constexpr auto LIBRARY_NAME = "Octane API Client";
  /**
   * @brief ライブラリバージョン。
   * @details
   * 命名規則はv{MAJOR}.{MINOR}.{PATCH}である。
   *
   */
  constexpr auto LIBRARY_VERSION = "v0.1.0";

  /** @brief 既定で使用されるAPIのトークン。*/
  constexpr auto DEFAULT_API_TOKEN = "mock";
  /** @brief 既定で使用されるAPIのオリジン。*/
  constexpr auto DEFAULT_API_ORIGIN = "http://localhost:3000";
  /** @brief 既定で使用されるAPIのベースURL。*/
  constexpr auto DEFAULT_API_BASE_URL = "/api/v1";
}; // namespace octane

#endif // OCTANE_API_CLIENT_CONFIG_H_