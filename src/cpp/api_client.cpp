/**
 * @file api-client.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief api_client.h の実装
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
    : bridge(
      new internal::Fetch(token, origin, baseUrl, new internal::HttpClient())),
      lastCheckedTime(0) {}

  ApiClient::~ApiClient() noexcept {
    if (connectionStatus.isConnected == true) {
      disconnectRoom(connectionStatus.id, connectionStatus.name);
    }
  }

  Result<Response, ErrorResponse> ApiClient::init() {
    auto result = bridge.init();
    if (!result) {
      return error(result.err());
    }
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
    Response response{};
    response.health  = checkHealthResult.get().health;
    response.message = checkHealthResult.get().message;
    return ok(response);
  }

  Result<HealthResult, ErrorResponse> ApiClient::checkHealth() {
    uint64_t now = std::chrono::duration_cast<std::chrono::seconds>(
                     std::chrono::system_clock::now().time_since_epoch())
                     .count();
    if (now - lastCheckedTime < 1800) {
      return ok(lastCheckedHealth);
    }

    auto healthResult = health();
    if (!healthResult) {
      return error(healthResult.err());
    }

    const auto& [health, message] = healthResult.get();
    if (health == Health::Faulty) {
      return makeError(ERR_SERVER_HEALTH_STATUS_FAULTY, message.value_or(""));
    }

    if (health != Health::Healthy && health != Health::Degraded) {
      std::abort();
    }
    lastCheckedHealth = healthResult.get();
    lastCheckedTime   = now;
    return ok(lastCheckedHealth);
  }

  Result<HealthResult, ErrorResponse> ApiClient::health() {
    auto result = bridge.healthGet();
    if (!result) {
      return error(result.err());
    }
    return ok(result.get());
  }

  Result<RoomId, ErrorResponse> ApiClient::createRoom(std::string_view name) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
    auto result = bridge.roomPost(name);
    if (!result) {
      return error(result.err());
    }
    auto response    = result.get();
    response.health  = checkHealthResult.get().health;
    response.message = checkHealthResult.get().message;
    return ok(response);
  }

  Result<Response, ErrorResponse> ApiClient::connectRoom(
    std::uint64_t id,
    std::string_view name) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
    auto result = bridge.roomIdPost(id, name, "connect");
    if (!result) {
      return error(result.err());
    }
    Response response{};
    response.health              = checkHealthResult.get().health;
    response.message             = checkHealthResult.get().message;
    connectionStatus.id          = id;
    connectionStatus.isConnected = true;
    connectionStatus.name        = name;
    return ok(response);
  }
  Result<Response, ErrorResponse> ApiClient::disconnectRoom(
    std::uint64_t id,
    std::string_view name) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
    auto result = bridge.roomIdPost(id, name, "disconnect");
    if (!result) {
      return error(result.err());
    }
    Response response{};
    response.health              = checkHealthResult.get().health;
    response.message             = checkHealthResult.get().message;
    connectionStatus.id          = 0;
    connectionStatus.isConnected = false;
    connectionStatus.name        = "";
    return ok(response);
  }

  Result<RoomStatus, ErrorResponse> ApiClient::getRoomStatus(
    std::optional<std::uint64_t> id) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
    if (!id.has_value() && !connectionStatus.isConnected) {
      return makeError(
        ERR_ROOM_ID_UNDEFINED,
        "Room id is undefined even though this device is disconnected from a room");
    }
    auto result
      = bridge.roomIdGet(id.has_value() ? id.value() : connectionStatus.id);
    if (!result) {
      return error(result.err());
    }
    auto response    = result.get();
    response.health  = checkHealthResult.get().health;
    response.message = checkHealthResult.get().message;
    return ok(response);
  }

  Result<Response, ErrorResponse> ApiClient::deleteRoom(
    std::optional<std::uint64_t> id) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
    if (!id.has_value() && !connectionStatus.isConnected) {
      return makeError(
        ERR_ROOM_ID_UNDEFINED,
        "Room id is undefined even though this device is disconnected from a room");
    }
    auto result
      = bridge.roomIdDelete(id.has_value() ? id.value() : connectionStatus.id);
    if (!result) {
      return error(result.err());
    }
    Response response{};
    response.health  = checkHealthResult.get().health;
    response.message = checkHealthResult.get().message;
    return ok(response);
  }

  Result<Content, ErrorResponse> ApiClient::getContent() {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
    if (!connectionStatus.isConnected) {
      return makeError(ERR_ROOM_DISCONNECTED,
                       "This device is disconnected from the room");
    }
    auto resultS = bridge.roomIdStatusGet(connectionStatus.id);
    if (!resultS) {
      return error(resultS.err());
    }
    Content content{};
    content.contentStatus = resultS.get().first;
    if (content.contentStatus.type == ContentType::MultiFile) {
      // TODO: zipなので解凍してあげる
    }
    auto result = bridge.roomIdContentGet(connectionStatus.id);
    if (!result) {
      return error(result.err());
    }
    content.data = result.get();
    // validate hash value
    std::vector<std::uint8_t> hashData;
    if (std::holds_alternative<std::vector<std::uint8_t>>(content.data)) {
      hashData = std::get<std::vector<std::uint8_t>>(content.data);
    } else if (std::holds_alternative<std::string>(content.data)) {
      std::string hashDataString = std::get<std::string>(content.data);
      hashData.resize(hashDataString.size());
      std::copy(hashDataString.begin(), hashDataString.end(), hashData.begin());
    } else {
      std::abort();
    }
    std::string hash = internal::generateHash(hashData);
    if (resultS.get().second != hash) {
      return makeError(ERR_CONTENT_HASH_MISMATCH,
                       "Content data doesn't match with its own hash value");
    }
    auto response    = content;
    response.health  = checkHealthResult.get().health;
    response.message = checkHealthResult.get().message;
    return ok(response);
  }

  Result<Response, ErrorResponse> ApiClient::deleteContent() {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
    if (!connectionStatus.isConnected) {
      return makeError(ERR_ROOM_DISCONNECTED,
                       "This device is disconnected from the room");
    }
    auto result = bridge.roomIdContentDelete(connectionStatus.id);
    if (!result) {
      return error(result.err());
    }
    Response response{};
    response.health  = checkHealthResult.get().health;
    response.message = checkHealthResult.get().message;
    return ok(response);
  }

  Result<Response, ErrorResponse> ApiClient::uploadContent(
    const Content& content) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
    if (!connectionStatus.isConnected) {
      return makeError(ERR_ROOM_DISCONNECTED,
                       "This device is disconnected from the room");
    }
    // convert input data into binary
    std::vector<std::uint8_t> contentData;
    if (content.contentStatus.type == ContentType::Clipboard
        || content.contentStatus.type == ContentType::File) {
      if (std::holds_alternative<std::vector<FileInfo>>(content.data)) {
        return makeError(
          ERR_CONTENT_TYPE_DATA_MISMATCH,
          "The specified type of content.contentStatus.type doesn't match content.data");
      }
      contentData = createBinary(content.data);
    } else if (content.contentStatus.type == ContentType::MultiFile) {
      if (!std::holds_alternative<std::vector<FileInfo>>(content.data)) {
        return makeError(
          ERR_CONTENT_TYPE_DATA_MISMATCH,
          "The specified type of content.contentStatus.type doesn't match content.data");
      }
      std::vector<FileInfo> MultiFiles
        = std::get<std::vector<FileInfo>>(content.data);
      for (auto file : MultiFiles) {
      }
      // TODO:ファイルそれぞれを圧縮して、最終的にcontentDataに格納する。
    } else {
      std::abort();
    }
    // generate hash value
    std::string hash = internal::generateHash(contentData);
    auto resultS     = bridge.roomIdStatusPut(
      connectionStatus.id, content.contentStatus, hash);
    if (!resultS) {
      return error(resultS.err());
    }
    auto result = bridge.roomIdContentPut(
      connectionStatus.id, contentData, content.contentStatus.mime);
    if (!result) {
      return error(result.err());
    }
    Response response{};
    response.health  = checkHealthResult.get().health;
    response.message = checkHealthResult.get().message;
    return ok(response);
  }
  std::vector<std::uint8_t> ApiClient::createBinary(
    const std::variant<std::string,
                       std::vector<uint8_t>,
                       std::vector<FileInfo>>& input) {
    assert(!std::holds_alternative<std::vector<FileInfo>>(input));
    if (std::holds_alternative<std::vector<std::uint8_t>>(input)) {
      return std::get<std::vector<std::uint8_t>>(input);
    }
    std::vector<std::uint8_t> outputBinary;
    std::string inputString = std::get<std::string>(input);
    outputBinary.resize(inputString.size());
    std::copy(inputString.begin(), inputString.end(), outputBinary.begin());
    return outputBinary;
  }
} // namespace octane