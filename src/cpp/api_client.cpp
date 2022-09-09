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
#include "include/internal/multi_file.h"

namespace octane {
  ApiClient::ApiClient(std::string_view token,
                       std::string_view origin,
                       std::string_view baseUrl)
    : bridge(
      new internal::Fetch(token, origin, baseUrl, new internal::HttpClient())),
      lastCheckedTime(0),
      connectionStatus(ConnectionStatus{
        .isConnected = false,
      }) {}

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

    Content content;

    auto status = bridge.roomIdStatusGet(connectionStatus.id);
    if (!status) {
      return error(status.err());
    }
    content.contentStatus = std::move(status.get().first);

    auto result = bridge.roomIdContentGet(connectionStatus.id);
    if (!result) {
      return error(result.err());
    }
    auto hash = internal::generateHash(result.get());
    if (status.get().second != hash) {
      return makeError(ERR_CONTENT_HASH_MISMATCH,
                       "Content data doesn't match with its own hash value");
    }

    if (content.contentStatus.type == ContentType::File) {
      content.data = std::move(result.get());
    } else if (content.contentStatus.type == ContentType::Clipboard) {
      std::string str;
      str.reserve(result.get().size());
      std::copy(
        result.get().begin(), result.get().end(), std::back_inserter(str));
      content.data = std::move(str);
    } else {
      auto data = internal::MultiFileDecompressor::decompress(result.get());
      if (!data) {
        return error(data.err());
      }
      content.data = std::move(data.get());
    }

    content.health  = checkHealthResult.get().health;
    content.message = checkHealthResult.get().message;

    return ok(std::move(content));
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

    const auto send = [&](const std::vector<std::uint8_t>& data)
      -> Result<Response, ErrorResponse> {
      std::string hash = internal::generateHash(data);
      auto resultS     = bridge.roomIdStatusPut(
        connectionStatus.id, content.contentStatus, hash);
      if (!resultS) {
        return error(resultS.err());
      }
      // TODO: mime関係の処理が歪すぎるのでどうにかしましょう
      std::string mime = content.contentStatus.mime;
      if (content.contentStatus.type == ContentType::Clipboard) {
        mime = "text/plain";
      } else if (content.contentStatus.type == ContentType::MultiFile) {
        mime = "application/x-7z-compressed";
      }
      auto result = bridge.roomIdContentPut(connectionStatus.id, data, mime);
      if (!result) {
        return error(result.err());
      }
      Response response{};
      response.health  = checkHealthResult.get().health;
      response.message = checkHealthResult.get().message;
      return ok(response);
    };

    if (content.contentStatus.type == ContentType::Clipboard
        || content.contentStatus.type == ContentType::File) {
      if (std::holds_alternative<std::string>(content.data)) {
        auto& str = std::get<std::string>(content.data);
        std::vector<std::uint8_t> data;
        data.reserve(str.size());
        std::copy(str.begin(), str.end(), std::back_inserter(data));
        return send(data);
      } else if (std::holds_alternative<std::vector<std::uint8_t>>(
                   content.data)) {
        return send(std::get<std::vector<std::uint8_t>>(content.data));
      } else if (std::holds_alternative<std::vector<FileInfo>>(content.data)) {
        return makeError(
          ERR_CONTENT_TYPE_DATA_MISMATCH,
          "The specified type of content.contentStatus.type doesn't match content.data");
      }
    } else if (content.contentStatus.type == ContentType::MultiFile) {
      if (!std::holds_alternative<std::vector<FileInfo>>(content.data)) {
        return makeError(
          ERR_CONTENT_TYPE_DATA_MISMATCH,
          "The specified type of content.contentStatus.type doesn't match content.data");
      }
      auto data = internal::MultiFileCompressor::compress(
        std::get<std::vector<FileInfo>>(content.data));
      if (!data) {
        return error(data.err());
      }
      return send(data.get());
    }

    // 到達不能コードのはず
    std::abort();
  }
} // namespace octane