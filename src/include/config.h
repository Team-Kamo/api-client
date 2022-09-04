/**
 * @file config.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief Define constants which are used on compiling.
 * @version 0.1
 * @date 2022-09-02
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef OCTANE_API_CLIENT_CONFIG_H_
#define OCTANE_API_CLIENT_CONFIG_H_

namespace octane {
  /** Library name. Octane API Client。*/
  constexpr auto LIBRARY_NAME = "Octane API Client";
  /**
   * @brief Library version.
   * @details
   * Naming conventions are v{MAJOR}.{MINOR}.{PATCH}
   *
   */
  constexpr auto LIBRARY_VERSION = "v0.1.0";

  /** @brief Default API token. */
  constexpr auto DEFAULT_API_TOKEN = "mock";
  /** @brief Default API origin. */
  constexpr auto DEFAULT_API_ORIGIN = "http://localhost:3000";
  /** @brief Default base URL.*/
  constexpr auto DEFAULT_API_BASE_URL = "/api/v1";
}; // namespace octane

#endif // OCTANE_API_CLIENT_CONFIG_H_