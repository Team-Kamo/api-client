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
  ApiClient::ApiClient(std::string_view token,
                       std::string_view origin,
                       std::string_view baseUrl)
    : bridge(token, origin, baseUrl), lastCheckedTime(0) {}

  ApiClient::~ApiClient() noexcept {}

  Result<std::optional<std::string>, ErrorResponse> ApiClient::init() {
    auto result = bridge.init();
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
      return error(
        makeError(ERR_SERVER_HEALTH_STATUS_FAULTY, message.value_or("")));
    }

    if (health != Health::Healthy && health != Health::Degraded) {
      std::abort();
    }
    lastCheckedTime = now;
    return ok(message);
  }

  Result<HealthResult, ErrorResponse> ApiClient::health() {
    auto result = bridge.healthGet();
    if (!result) {
      return error(result.err());
    }
    return ok(result.get());
  }

  Result<std::string, ErrorResponse> ApiClient::createRoom(
    std::string_view name) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
    auto result = bridge.roomPost(name);
    if (!result) {
      return error(result.err());
    }
    return ok(result.get());
  }

  Result<RoomStatus, ErrorResponse> ApiClient::getRoomStatus(
    std::string_view id) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
    auto result = bridge.roomIdGet(id);
    if (!result) {
      return error(result.err());
    }
    return ok(result.get());
  }

  Result<_, ErrorResponse> ApiClient::deleteRoom(std::string_view id) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
    auto result = bridge.roomIdDelete(id);
    if (!result) {
      return error(result.err());
    }
    return ok();
  }

  Result<Content, ErrorResponse> ApiClient::getContent(std::string_view id) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
    auto result = bridge.roomIdContentGet(id);
    if (!result) {
      return error(result.err());
    }
    return ok(result.get());
  }

  Result<_, ErrorResponse> ApiClient::deleteContent(std::string_view id) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
    auto result = bridge.roomIdContentDelete(id);
    if (!result) {
      return error(result.err());
    }
    return ok();
  }

  Result<_, ErrorResponse> ApiClient::uploadContent(std::string_view id,
                                                    const Content& content) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
    auto result = bridge.roomIdContentPut(id, content);
    if (!result) {
      return error(result.err());
    }
    return ok();
  }

  Result<_, ErrorResponse> ApiClient::connectRoom(std::string_view id,
                                                  std::string_view name) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
    auto result = bridge.roomIdPost(id, name);
    if (!result) {
      return error(result.err());
    }
    return ok();
  }

  Result<internal::ContentStatus, ErrorResponse> ApiClient::getContentStatus(
    std::string_view id) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
    auto result = bridge.roomIdStatusGet(id);
    if (!result) {
      return error(result.err());
    }
    return ok(result.get());
  }

  Result<_, ErrorResponse> ApiClient::uploadContentStatus(
    std::string_view id,
    const internal::ContentStatus& contentStatus) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
    // TODO: Hash値をApiClient側で自動的に生成する
    auto result = bridge.roomIdStatusPut(id, contentStatus);
    if (!result) {
      return error(result.err());
    }
    return ok(result.get());
  }

  Result<_, ErrorResponse> ApiClient::deleteContentStatus(std::string_view id) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
    auto result = bridge.roomIdStatusDelete(id);
    if (!result) {
      return error(result.err());
    }
    return ok(result.get());
  }
} // namespace octane