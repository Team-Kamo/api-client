/**
 * @file api-client.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-30
 *
 * api-client.hの実装。
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "include/api_client.h"

#include <chrono>

#include "include/error_code.h"

namespace octane {
  ApiClient::ApiClient(std::string_view token, std::string_view origin, std::string_view baseUrl)
    : fetch(token, origin, baseUrl), lastCheckedTime(0) {}
  ApiClient::~ApiClient() noexcept {}
  Result<std::optional<std::string>, ErrorResponse> ApiClient::init() {
    auto result = fetch.init();
    if (!result) {
      return error(result.err());
    }
    return checkHealth();
  }

  Result<std::optional<std::string>, ErrorResponse> ApiClient::checkHealth() {
    uint64_t now = std::chrono::duration_cast<std::chrono::seconds>(
                     std::chrono::system_clock::now().time_since_epoch())
                     .count();
    if (now - lastCheckedTime < 1800) {
      return ok(std::nullopt);
    }

    auto healthResult = health();
    if (!healthResult) {
      return error(healthResult.err());
    }

    const auto& [health, message] = healthResult.get();
    if (health == Health::Faulty) {
      return error(ErrorResponse{
        .code   = ERR_API_CLIENT_INITIALIZATION_FAILED,
        .reason = message.value_or(""),
      });
    }

    if (health != Health::Healthy && health != Health::Degraded) {
      std::abort();
    }
    lastCheckedTime = now;
    return ok(message);
  }

  Result<HealthResult, ErrorResponse> ApiClient::health() {
  }

  Result<std::string, ErrorResponse> ApiClient::createRoom(
    std::string_view name) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
  }

  Result<RoomStatus, ErrorResponse> ApiClient::getRoomStatus(
    std::string_view id) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
  }

  Result<_, ErrorResponse> ApiClient::deleteRoom(std::string_view id) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
  }

  Result<Content, ErrorResponse> ApiClient::getContent(std::string_view id) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
  }

  Result<_, ErrorResponse> ApiClient::deleteContent(std::string_view id) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
  }

  Result<_, ErrorResponse> ApiClient::uploadContent(std::string_view id,
                                                    const Content& content) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
  }

  Result<_, ErrorResponse> ApiClient::connectRoom(std::string_view id,
                                                  std::string_view name) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
  }

  Result<ContentStatus, ErrorResponse> ApiClient::getContentStatus(
    std::string_view id) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
  }

  Result<_, ErrorResponse> ApiClient::uploadContentStatus(
    std::string_view id,
    const ContentStatus& contentStatus) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
  }

  Result<_, ErrorResponse> ApiClient::deleteContentStatus(std::string_view id) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
  }
} // namespace octane