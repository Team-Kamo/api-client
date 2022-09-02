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
    : bridge(new internal::Fetch(token, origin, baseUrl, new internal::HttpClient())), lastCheckedTime(0) {}

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

  Result<Content, ErrorResponse> ApiClient::getContent(std::string_view id,
                                                       std::string_view name) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
    auto resultC = bridge.roomIdPost(id, name);
    if (!resultC) {
      return error(resultC.err());
    }
    auto resultS = bridge.roomIdStatusGet(id);
    if (!resultS) {
      return error(resultS.err());
    }
    Content content{};
    content.contentStatus = resultS.get();
    auto result           = bridge.roomIdContentGet(id);
    if (!result) {
      return error(result.err());
    }
    content.data = result.get();
    return ok(content);
  }

  Result<_, ErrorResponse> ApiClient::deleteContent(std::string_view id,
                                                    std::string_view name) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
    auto resultC = bridge.roomIdPost(id, name);
    if (!resultC) {
      return error(resultC.err());
    }
    auto resultS = bridge.roomIdStatusDelete(id);
    if (!resultS) {
      return error(resultS.err());
    }
    auto result = bridge.roomIdContentDelete(id);
    if (!result) {
      return error(result.err());
    }
    return ok();
  }

  Result<_, ErrorResponse> ApiClient::uploadContent(std::string_view id,
                                                    std::string_view name,
                                                    const Content& content) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
    auto resultC = bridge.roomIdPost(id, name);
    if (!resultC) {
      return error(resultC.err());
    }
    std::vector<std::uint8_t> hashData;
    if (std::holds_alternative<std::vector<std::uint8_t>>(content.data)) {
      hashData = std::get<std::vector<std::uint8_t>>(content.data);
    } else if (std::holds_alternative<std::string>(content.data)) {
      std::string hashDataString = std::get<std::string>(content.data);
      hashData.reserve(hashDataString.size());
      std::copy(hashDataString.begin(), hashDataString.end(), hashData.begin());
    } else {
      return makeError(ERR_INVALID_REQUEST,
                       "content.data type is not binary or string");
    }
    std::string hash = internal::generateHash(hashData);
    auto resultS     = bridge.roomIdStatusPut(id, content.contentStatus, hash);
    if (!resultS) {
      return error(resultS.err());
    }
    auto result
      = bridge.roomIdContentPut(id, content.data, content.contentStatus.mime);
    if (!result) {
      return error(result.err());
    }
    return ok();
  }
} // namespace octane