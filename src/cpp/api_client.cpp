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

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <chrono>

#include "include/error_code.h"

namespace octane {
  ApiClient::ApiClient(std::string_view token,
                       std::string_view origin,
                       std::string_view baseUrl)
    : fetch(token, origin, baseUrl, new internal::HttpClient()),
      lastCheckedTime(0) {}

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
    auto response = fetch.request(octane::internal::HttpMethod::Get, "/health");
    if (!response) {
      return error(response.err());
    }
    if (!std::holds_alternative<rapidjson::Document>(response.get())) {
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, json not returned");
    }
    rapidjson::Document& json = std::get<rapidjson::Document>(response.get());
    HealthResult healthResult{};
    if (!json.HasMember("health"))
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, json doesn't have member 'health'");
    if (!json.HasMember("message"))
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, json doesn't have member 'message'");
    if (!json["health"].IsString())
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, member 'health' type is not string");
    if (!json["message"].IsString())
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, member 'message' type is not string");
    if (json["health"].GetString() == "healthy") {
      healthResult.health = Health::Healthy;
    } else if (json["health"].GetString() == "degraded") {
      healthResult.health = Health::Degraded;
    } else if (json["health"].GetString() == "faulty") {
      healthResult.health = Health::Faulty;
    } else {
      return makeError(
        ERR_INVALID_RESPONSE,
        "Invalid response, member 'health' is not 'healthy', 'degraded' or 'faulty'");
    }
    healthResult.message = json["message"].GetString();
    return ok(healthResult);
  }

  Result<std::string, ErrorResponse> ApiClient::createRoom(
    std::string_view name) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
    rapidjson::StringBuffer buffer;
    rapidjson::Writer writer(buffer);
    writer.StartObject();
    writer.Key("name");
    writer.String(std::string(name).c_str());
    writer.EndObject();
    rapidjson::Document uploadJson;
    uploadJson.Accept(writer);
    auto response
      = fetch.request(octane::internal::HttpMethod::Post, "/room", uploadJson);
    if (!response) {
      return error(response.err());
    }
    if (!std::holds_alternative<rapidjson::Document>(response.get())) {
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, json not returned");
    }
    rapidjson::Document& json = std::get<rapidjson::Document>(response.get());
    std::string id;
    if (!json.HasMember("id"))
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, json doesn't have member 'id'");
    if (!json["id"].IsString())
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, member 'id' type is not string");
    id = json["id"].GetString();
    return ok(id);
  }

  Result<RoomStatus, ErrorResponse> ApiClient::getRoomStatus(
    std::string_view id) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
    auto response = fetch.request(octane::internal::HttpMethod::Get,
                                  "/room/" + std::string(id));
    if (!response) {
      return error(response.err());
    }
    if (!std::holds_alternative<rapidjson::Document>(response.get())) {
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, json not returned");
    }
    rapidjson::Document& json = std::get<rapidjson::Document>(response.get());
    RoomStatus roomStatus{};
    std::vector<Device> devices;
    if (!json.HasMember("name"))
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, json doesn't have member 'name'");
    if (!json.HasMember("devices"))
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, json doesn't have member 'devices'");
    if (!json["name"].IsString())
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, member 'name' type is not string");
    if (!json["devices"].IsArray())
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, member 'devices' type is not array");
    roomStatus.name = json["name"].GetString();
    for (int i = 0; i < json["devices"].Size(); i++) {
      auto& jsonDevice = json["devices"][i];
      Device device{};
      if (!jsonDevice.HasMember("name"))
        return makeError(
          ERR_INVALID_RESPONSE,
          "Invalid response, object of member 'devices' doesn't have member 'name'");
      if (!jsonDevice.HasMember("timestamp"))
        return makeError(
          ERR_INVALID_RESPONSE,
          "Invalid response, object of member 'devices' doesn't have member 'timestamp'");
      if (!jsonDevice["name"].IsString())
        return makeError(
          ERR_INVALID_RESPONSE,
          "Invalid response, 'devices' member 'name' type is not string");
      if (!jsonDevice["timestamp"].IsUint64())
        return makeError(
          ERR_INVALID_RESPONSE,
          "Invalid response, 'devices' member 'timestamp' type is not uint64_t");
      device.name      = jsonDevice["name"].GetString();
      device.timestamp = jsonDevice["timestamp"].GetUint64();
      devices.push_back(device);
    }
    roomStatus.devices = devices;
    return ok(roomStatus);
  }

  Result<_, ErrorResponse> ApiClient::deleteRoom(std::string_view id) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
    auto response = fetch.request(octane::internal::HttpMethod::Delete,
                                  "/room/" + std::string(id));
    if (!response) {
      return error(response.err());
    }
    return ok();
  }

  Result<Content, ErrorResponse> ApiClient::getContent(std::string_view id) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
    auto response = fetch.request(octane::internal::HttpMethod::Get,
                                  "/room/" + std::string(id) + "/content");
    if (!response) {
      return error(response.err());
    }
    if (!std::holds_alternative<std::vector<uint8_t>>(response.get())) {
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, binary not returned");
    }
    Content content{};
    content.data = std::get<std::vector<uint8_t>>(response.get());
    // TODO: contentに他の中身を入れる
    return ok();
  }

  Result<_, ErrorResponse> ApiClient::deleteContent(std::string_view id) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
    auto response = fetch.request(octane::internal::HttpMethod::Delete,
                                  "/room/" + std::string(id) + "/content");
    if (!response) {
      return error(response.err());
    }
    // TODO: 全てのresponseについてこれをやる
    // if (response.get().status >= 200 && response.get().status < 300) {
    // }

    return ok();
  }

  Result<_, ErrorResponse> ApiClient::uploadContent(std::string_view id,
                                                    const Content& content) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
    std::vector<uint8_t> data;
    if (std::holds_alternative<std::vector<uint8_t>>(content.data)) {
      data = std::get<std::vector<uint8_t>>(content.data);
    } else if (std::holds_alternative<std::string>(content.data)) {
      const std::string& contentData = std::get<std::string>(content.data);
      data.reserve(contentData.size());
      std::copy(contentData.begin(), contentData.end(), data.begin());
    } else {
      return makeError(ERR_INVALID_REQUEST,
                       "content.data is not string or binary");
    }
    auto response = fetch.request(octane::internal::HttpMethod::Put,
                                  "/room/" + std::string(id) + "/content",
                                  content.mime,
                                  data);
    if (!response) {
      return error(response.err());
    }
    return ok();
  }

  Result<_, ErrorResponse> ApiClient::connectRoom(std::string_view id,
                                                  std::string_view name) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
    rapidjson::StringBuffer buffer;
    rapidjson::Writer writer(buffer);
    writer.StartObject();
    writer.Key("name");
    writer.String(std::string(name).c_str());
    writer.EndObject();
    rapidjson::Document uploadJson;
    uploadJson.Accept(writer);
    auto response = fetch.request(octane::internal::HttpMethod::Post,
                                  "/room/" + std::string(id),
                                  uploadJson);
    if (!response) {
      return error(response.err());
    }
    return ok();
  }

  Result<ContentStatus, ErrorResponse> ApiClient::getContentStatus(
    std::string_view id) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
    auto response = fetch.request(octane::internal::HttpMethod::Get,
                                  "/room/" + std::string(id) + "/status");
    if (!response) {
      return error(response.err());
    }
    if (!std::holds_alternative<rapidjson::Document>(response.get())) {
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, json not returned");
    }
    rapidjson::Document& json = std::get<rapidjson::Document>(response.get());
    ContentStatus status{};
    if (!json.HasMember("device"))
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, json doesn't have member 'device'");
    if (!json.HasMember("timestamp"))
      return makeError(
        ERR_INVALID_RESPONSE,
        "Invalid response, json doesn't have member 'timestamp'");
    if (!json.HasMember("type"))
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, json doesn't have member 'type'");
    if (!json.HasMember("mime"))
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, json doesn't have member 'mime'");
    if (!json.HasMember("hash"))
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, json doesn't have member 'hash'");
    if (!json["device"].IsString())
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, member 'device' type is not string");
    if (!json["timestamp"].IsUint64())
      return makeError(
        ERR_INVALID_RESPONSE,
        "Invalid response, member 'timestamp' type is not uint64_t");
    if (!json["type"].IsString())
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, member 'type' type is not string");
    if (!json["mime"].IsString())
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, member 'mime' type is not string");
    if (!json["hash"].IsString())
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, member 'hash' type is not string");
    status.device    = json["device"].GetString();
    status.timestamp = json["timestamp"].GetUint64();
    if (json["type"].GetString() == "file") {
      status.type = ContentType::File;
    } else if (json["type"].GetString() == "clipboard") {
      status.type = ContentType::Clipboard;
    } else {
      return makeError(
        ERR_INVALID_RESPONSE,
        "Invalid response, json member 'type' is not 'file' or 'clipboard'");
    }
    if (status.type == ContentType::Clipboard) return ok(status);
    if (!json.HasMember("name"))
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, json doesn't have member 'name'");
    if (!json["name"].IsString())
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, member 'name' type is not string");
    return ok(status);
  }

  Result<_, ErrorResponse> ApiClient::uploadContentStatus(
    std::string_view id,
    const ContentStatus& contentStatus) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }

    rapidjson::StringBuffer buffer;
    rapidjson::Writer writer(buffer);

    writer.StartObject();
    writer.Key("device");
    writer.String(contentStatus.device.c_str());
    writer.Key("timestamp");
    writer.Uint64(contentStatus.timestamp);
    if (contentStatus.type == ContentType::File) {
      writer.Key("type");
      writer.String("file");
      writer.Key("name");
      writer.String(contentStatus.name.value_or("file").c_str());
    } else if (contentStatus.type == ContentType::Clipboard) {
      writer.Key("type");
      writer.String("clipboard");
    } else {
      return makeError(
        ERR_INVALID_REQUEST,
        "Invalid request, contentStatus.type is not 'File' or 'Clipboard'");
    }
    writer.Key("mime");
    writer.String(contentStatus.mime.c_str());

    // TODO: Hash値をApiClient側で自動的に生成する
    writer.Key("hash");
    writer.String(contentStatus.hash.c_str());

    writer.EndObject();

    rapidjson::Document uploadJson;
    uploadJson.Accept(writer);

    auto response = fetch.request(octane::internal::HttpMethod::Post,
                                  "/room/" + std::string(id) + "/status",
                                  uploadJson);
    if (!response) {
      return error(response.err());
    }
    return ok();
  }

  Result<_, ErrorResponse> ApiClient::deleteContentStatus(std::string_view id) {
    const auto checkHealthResult = checkHealth();
    if (!checkHealthResult) {
      return error(checkHealthResult.err());
    }
    auto response = fetch.request(octane::internal::HttpMethod::Delete,
                                  "/room/" + std::string(id) + "/status");
    if (!response) {
      return error(response.err());
    }
    return ok();
  }
} // namespace octane