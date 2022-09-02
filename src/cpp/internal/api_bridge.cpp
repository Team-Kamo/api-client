/**
 * @file api_bridge.cpp
 * @author soon (kento.soon@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-09-01
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "include/internal/api_bridge.h"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "include/error_code.h"

namespace octane::internal {
  ApiBridge::ApiBridge(FetchBase* fetch) : fetch(fetch) {}
  Result<_, ErrorResponse> ApiBridge::init() {
    return fetch->init();
  }
  Result<HealthResult, ErrorResponse> ApiBridge::healthGet() {
    using namespace std::string_literals;
    auto response = fetch->request(internal::HttpMethod::Get, "/health");
    if (!response) {
      return error(response.err());
    }
    if (auto err = checkStatusCode(response.get())) {
      return error(err.value());
    }
    if (!std::holds_alternative<rapidjson::Document>(response.get().body)) {
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, json not returned");
    }
    rapidjson::Document& json
      = std::get<rapidjson::Document>(response.get().body);
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
    if (json["health"].GetString() == "healthy"s) {
      healthResult.health = Health::Healthy;
    } else if (json["health"].GetString() == "degraded"s) {
      healthResult.health = Health::Degraded;
    } else if (json["health"].GetString() == "faulty"s) {
      healthResult.health = Health::Faulty;
    } else {
      return makeError(
        ERR_INVALID_RESPONSE,
        "Invalid response, member 'health' is not 'healthy', 'degraded' or 'faulty' "s
          + json["health"].GetString() + " was passed");
    }
    healthResult.message = json["message"].GetString();
    return ok(healthResult);
  }
  Result<std::string, ErrorResponse> ApiBridge::roomPost(
    std::string_view name) {
    // ここにおそらくテストで引っ掛かっているようなところがあるかも知れません。
    // これかもhttps://techoverflow.net/2020/01/13/how-to-fix-rapidjson-assertion-hasroot_-failed/
    rapidjson::StringBuffer buffer;
    rapidjson::Writer writer(buffer);
    writer.StartObject();
    writer.Key("name");
    writer.String(std::string(name).c_str());
    writer.EndObject();
    rapidjson::Document uploadJson;
    uploadJson.Accept(writer);
    auto response
      = fetch->request(internal::HttpMethod::Post, "/room", uploadJson);
    if (!response) {
      return error(response.err());
    }
    if (auto err = checkStatusCode(response.get())) {
      return error(err.value());
    }
    if (!std::holds_alternative<rapidjson::Document>(response.get().body)) {
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, json not returned");
    }
    rapidjson::Document& json
      = std::get<rapidjson::Document>(response.get().body);
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
  Result<RoomStatus, ErrorResponse> ApiBridge::roomIdGet(std::string_view id) {
    auto response
      = fetch->request(internal::HttpMethod::Get, "/room/" + std::string(id));
    if (!response) {
      return error(response.err());
    }
    if (auto err = checkStatusCode(response.get())) {
      return error(err.value());
    }
    if (!std::holds_alternative<rapidjson::Document>(response.get().body)) {
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, json not returned");
    }
    rapidjson::Document& json
      = std::get<rapidjson::Document>(response.get().body);
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
  Result<_, ErrorResponse> ApiBridge::roomIdDelete(std::string_view id) {
    auto response = fetch->request(internal::HttpMethod::Delete,
                                   "/room/" + std::string(id));
    if (!response) {
      return error(response.err());
    }
    if (auto err = checkStatusCode(response.get())) {
      return error(err.value());
    }
    return ok();
  }
  Result<_, ErrorResponse> ApiBridge::roomIdPost(std::string_view id,
                                                 std::string_view name) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer writer(buffer);
    writer.StartObject();
    writer.Key("name");
    writer.String(std::string(name).c_str());
    writer.EndObject();
    rapidjson::Document uploadJson;
    uploadJson.Accept(writer);
    auto response = fetch->request(
      internal::HttpMethod::Post, "/room/" + std::string(id), uploadJson);
    if (!response) {
      return error(response.err());
    }
    if (auto err = checkStatusCode(response.get())) {
      return error(err.value());
    }
    return ok();
  }
  Result<std::variant<std::string, std::vector<std::uint8_t>>, ErrorResponse>
  ApiBridge::roomIdContentGet(std::string_view id) {
    auto response = fetch->request(internal::HttpMethod::Get,
                                   "/room/" + std::string(id) + "/content");
    if (!response) {
      return error(response.err());
    }
    if (auto err = checkStatusCode(response.get())) {
      return error(err.value());
    }
    if (!std::holds_alternative<std::vector<std::uint8_t>>(
          response.get().body)) {
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, binary not returned");
    }
    return ok(std::get<std::vector<std::uint8_t>>(response.get().body));
  }
  Result<_, ErrorResponse> ApiBridge::roomIdContentDelete(std::string_view id) {
    auto response = fetch->request(internal::HttpMethod::Delete,
                                   "/room/" + std::string(id) + "/content");
    if (!response) {
      return error(response.err());
    }
    if (auto err = checkStatusCode(response.get())) {
      return error(err.value());
    }
    return ok();
  }
  Result<_, ErrorResponse> ApiBridge::roomIdContentPut(
    std::string_view id,
    const std::variant<std::string, std::vector<std::uint8_t>>& contentData,
    std::string_view mime) {
    std::vector<std::uint8_t> data;
    if (std::holds_alternative<std::vector<std::uint8_t>>(contentData)) {
      data = std::get<std::vector<std::uint8_t>>(contentData);
    } else if (std::holds_alternative<std::string>(contentData)) {
      const std::string& stringData = std::get<std::string>(contentData);
      data.reserve(stringData.size());
      std::copy(stringData.begin(), stringData.end(), data.begin());
    } else {
      return makeError(ERR_INVALID_REQUEST,
                       "contentData is not string or binary");
    }
    auto response = fetch->request(internal::HttpMethod::Put,
                                   "/room/" + std::string(id) + "/content",
                                   mime,
                                   data);
    if (!response) {
      return error(response.err());
    }
    if (auto err = checkStatusCode(response.get())) {
      return error(err.value());
    }
    return ok();
  }
  Result<ContentStatus, ErrorResponse> ApiBridge::roomIdStatusGet(
    std::string_view id) {
    using namespace std::string_literals;
    auto response = fetch->request(internal::HttpMethod::Get,
                                   "/room/" + std::string(id) + "/status");
    if (!response) {
      return error(response.err());
    }
    if (auto err = checkStatusCode(response.get())) {
      return error(err.value());
    }
    if (!std::holds_alternative<rapidjson::Document>(response.get().body)) {
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, json not returned");
    }
    rapidjson::Document& json
      = std::get<rapidjson::Document>(response.get().body);
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
    if (json["type"].GetString() == "file"s) {
      status.type = ContentType::File;
    } else if (json["type"].GetString() == "clipboard"s) {
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
  Result<_, ErrorResponse> ApiBridge::roomIdStatusDelete(std::string_view id) {
    auto response = fetch->request(internal::HttpMethod::Delete,
                                   "/room/" + std::string(id) + "/status");
    if (!response) {
      return error(response.err());
    }
    if (auto err = checkStatusCode(response.get())) {
      return error(err.value());
    }
    return ok();
  }
  Result<_, ErrorResponse> ApiBridge::roomIdStatusPut(
    std::string_view id,
    const ContentStatus& contentStatus,
    std::string_view hash) {
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
    writer.Key("hash");
    writer.String(std::string(hash).c_str());

    writer.EndObject();

    rapidjson::Document uploadJson;
    uploadJson.Accept(writer);

    auto response = fetch->request(internal::HttpMethod::Post,
                                   "/room/" + std::string(id) + "/status",
                                   uploadJson);
    if (!response) {
      return error(response.err());
    }
    if (auto err = checkStatusCode(response.get())) {
      return error(err.value());
    }
    return ok();
  }
  std::optional<error_t<ErrorResponse>> ApiBridge::checkStatusCode(
    internal::FetchResponse& response) {
    if (200 <= response.statusCode && response.statusCode < 300)
      return std::nullopt;
    if (!std::holds_alternative<rapidjson::Document>(response.body)) {
      return makeError(
        ERR_INVALID_RESPONSE,
        "Invalid response, json not returned. Maybe error at cdn?");
    }
    rapidjson::Document& json = std::get<rapidjson::Document>(response.body);
    if (!json.HasMember("code"))
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, json doesn't have member 'code'");
    if (!json.HasMember("reason"))
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, json doesn't have member 'message'");
    if (!json["code"].IsString())
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, member 'code' type is not string");
    if (!json["reason"].IsString())
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, member 'message' type is not string");
    return makeError(json["code"].GetString(), json["reason"].GetString());
  }
} // namespace octane::internal