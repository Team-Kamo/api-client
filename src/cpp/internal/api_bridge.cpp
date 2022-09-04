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
#include <rapidjson/schema.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <iostream>
#include <map>

#include "include/error_code.h"
#include "include/internal/api_schema.h"

namespace octane::internal {
  namespace {
    std::optional<ErrorResponse> verifyJson(const rapidjson::Document& json,
                                            std::string_view schema) {
      rapidjson::Document sd;
      assert(!sd.Parse(schema.data(), schema.size()).HasParseError());
      rapidjson::SchemaDocument schemaDoc(sd);
      rapidjson::SchemaValidator validator(schemaDoc);

      if (json.Accept(validator)) {
        return std::nullopt;
      }

      std::string msg;

      rapidjson::StringBuffer buf;
      validator.GetInvalidSchemaPointer().StringifyUriFragment(buf);
      msg += "\n\t\tInvalid schema: ";
      msg += buf.GetString();
      msg += "\n\t\tInvalid keyword: ";
      msg += validator.GetInvalidSchemaKeyword();

      buf.Clear();
      validator.GetInvalidDocumentPointer().StringifyUriFragment(buf);
      msg += "\n\t\tInvalid document: ";
      msg += buf.GetString();

      return ErrorResponse{
        .code   = ERR_INVALID_RESPONSE,
        .reason = msg,
      };
    }
  } // namespace

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
      return err.value();
    }
    if (!std::holds_alternative<rapidjson::Document>(response.get().body)) {
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, json not returned");
    }
    rapidjson::Document& json
      = std::get<rapidjson::Document>(response.get().body);

    if (auto err = verifyJson(json, SCHEMA_HEALTH_GET)) {
      return error(err.value());
    }

    static const std::map<std::string, Health> healthMap = {
      { "healthy", Health::Healthy },
      { "degraded", Health::Degraded },
      { "faulty", Health::Faulty },
    };

    return ok(HealthResult{
      .health  = healthMap.at(json["health"].GetString()),
      .message = json["message"].GetString(),
    });
  }
  Result<RoomId, ErrorResponse> ApiBridge::roomPost(std::string_view name) {
    rapidjson::Document uploadJson(rapidjson::kObjectType);
    uploadJson.AddMember("name",
                         rapidjson::StringRef(name.data(), name.size()),
                         uploadJson.GetAllocator());
    auto response
      = fetch->request(internal::HttpMethod::Post, "/room", uploadJson);
    if (!response) {
      return error(response.err());
    }
    if (auto err = checkStatusCode(response.get())) {
      return err.value();
    }
    if (!std::holds_alternative<rapidjson::Document>(response.get().body)) {
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, json not returned");
    }
    rapidjson::Document& json
      = std::get<rapidjson::Document>(response.get().body);
    if (auto err = verifyJson(json, SCHEMA_ROOM_POST)) {
      return error(err.value());
    }
    return ok(RoomId{ .id = json["id"].GetUint64() });
  }
  Result<RoomStatus, ErrorResponse> ApiBridge::roomIdGet(std::uint64_t id) {
    auto response = fetch->request(internal::HttpMethod::Get,
                                   "/room/" + std::to_string(id));
    if (!response) {
      return error(response.err());
    }
    if (auto err = checkStatusCode(response.get())) {
      return err.value();
    }
    if (!std::holds_alternative<rapidjson::Document>(response.get().body)) {
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, json not returned");
    }
    rapidjson::Document& json
      = std::get<rapidjson::Document>(response.get().body);

    if (auto err = verifyJson(json, SCHEMA_ROOM_ID_GET)) {
      return error(err.value());
    }

    std::vector<Device> devices;
    for (const auto& device : json["devices"].GetArray()) {
      devices.push_back(Device{
        .name      = device["name"].GetString(),
        .timestamp = device["timestamp"].GetUint64(),
      });
    }
    return ok(RoomStatus{ .name    = json["name"].GetString(),
                          .devices = devices,
                          .id      = json["id"].GetUint64() });
  }
  Result<_, ErrorResponse> ApiBridge::roomIdDelete(std::uint64_t id) {
    auto response = fetch->request(internal::HttpMethod::Delete,
                                   "/room/" + std::to_string(id));
    if (!response) {
      return error(response.err());
    }
    if (auto err = checkStatusCode(response.get())) {
      return err.value();
    }
    return ok();
  }
  Result<_, ErrorResponse> ApiBridge::roomIdPost(std::uint64_t id,
                                                 std::string_view name) {
    rapidjson::Document uploadJson(rapidjson::kObjectType);
    uploadJson.AddMember("name",
                         rapidjson::StringRef(name.data(), name.size()),
                         uploadJson.GetAllocator());
    auto response = fetch->request(
      internal::HttpMethod::Post, "/room/" + std::to_string(id), uploadJson);
    if (!response) {
      return error(response.err());
    }
    if (auto err = checkStatusCode(response.get())) {
      return err.value();
    }
    return ok();
  }
  Result<std::vector<std::uint8_t>, ErrorResponse> ApiBridge::roomIdContentGet(
    std::uint64_t id) {
    auto response = fetch->request(internal::HttpMethod::Get,
                                   "/room/" + std::to_string(id) + "/content");
    if (!response) {
      return error(response.err());
    }
    if (auto err = checkStatusCode(response.get())) {
      return err.value();
    }
    if (!std::holds_alternative<std::vector<std::uint8_t>>(
          response.get().body)) {
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, binary not returned");
    }
    return ok(std::get<std::vector<std::uint8_t>>(response.get().body));
  }
  Result<_, ErrorResponse> ApiBridge::roomIdContentDelete(std::uint64_t id) {
    auto response = fetch->request(internal::HttpMethod::Delete,
                                   "/room/" + std::to_string(id) + "/content");
    if (!response) {
      return error(response.err());
    }
    if (auto err = checkStatusCode(response.get())) {
      return err.value();
    }
    return ok();
  }
  Result<_, ErrorResponse> ApiBridge::roomIdContentPut(
    std::uint64_t id,
    const std::variant<std::string, std::vector<std::uint8_t>>& contentData,
    std::string_view mime) {
    std::vector<std::uint8_t> data;
    if (std::holds_alternative<std::vector<std::uint8_t>>(contentData)) {
      data = std::get<std::vector<std::uint8_t>>(contentData);
    } else if (std::holds_alternative<std::string>(contentData)) {
      const std::string& stringData = std::get<std::string>(contentData);
      data.resize(stringData.size());
      std::copy(stringData.begin(), stringData.end(), data.begin());
    } else {
      std::abort();
    }
    auto response = fetch->request(internal::HttpMethod::Put,
                                   "/room/" + std::to_string(id) + "/content",
                                   mime,
                                   data);
    if (!response) {
      return error(response.err());
    }
    if (auto err = checkStatusCode(response.get())) {
      return err.value();
    }
    return ok();
  }
  Result<ContentStatus, ErrorResponse> ApiBridge::roomIdStatusGet(
    std::uint64_t id) {
    using namespace std::string_literals;
    auto response = fetch->request(internal::HttpMethod::Get,
                                   "/room/" + std::to_string(id) + "/status");
    if (!response) {
      return error(response.err());
    }
    if (auto err = checkStatusCode(response.get())) {
      return err.value();
    }
    if (!std::holds_alternative<rapidjson::Document>(response.get().body)) {
      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, json not returned");
    }
    rapidjson::Document& json
      = std::get<rapidjson::Document>(response.get().body);

    if (auto err = verifyJson(json, SCHEMA_ROOM_ID_STATUS_GET)) {
      return error(err.value());
    }

    static const std::map<std::string, ContentType> typeMap = {
      { "file", ContentType::File },
      { "clipboard", ContentType::Clipboard },
    };

    ContentStatus status{
      .device    = json["device"].GetString(),
      .timestamp = json["timestamp"].GetUint64(),
      .type      = typeMap.at(json["type"].GetString()),
      .mime      = json["mime"].GetString(),
    };
    if (status.type == ContentType::Clipboard) {
      status.name = json["name"].GetString();
    }
    return ok(std::move(status));
  }
  Result<_, ErrorResponse> ApiBridge::roomIdStatusDelete(std::uint64_t id) {
    auto response = fetch->request(internal::HttpMethod::Delete,
                                   "/room/" + std::to_string(id) + "/status");
    if (!response) {
      return error(response.err());
    }
    if (auto err = checkStatusCode(response.get())) {
      return err.value();
    }
    return ok();
  }
  Result<_, ErrorResponse> ApiBridge::roomIdStatusPut(
    std::uint64_t id,
    const ContentStatus& contentStatus,
    std::string_view hash) {
    rapidjson::Document uploadJson(rapidjson::kObjectType);
    uploadJson.AddMember("device",
                         rapidjson::StringRef(contentStatus.device.data(),
                                              contentStatus.device.size()),
                         uploadJson.GetAllocator());
    uploadJson.AddMember("timestamp",
                         rapidjson::Value().SetUint64(contentStatus.timestamp),
                         uploadJson.GetAllocator());
    if (contentStatus.type == ContentType::File) {
      uploadJson.AddMember("type", "file", uploadJson.GetAllocator());
      uploadJson.AddMember(
        "name",
        rapidjson::StringRef(contentStatus.name.value_or("file").data(),
                             contentStatus.name.value_or("file").size()),
        uploadJson.GetAllocator());
    } else if (contentStatus.type == ContentType::Clipboard) {
      uploadJson.AddMember("type", "clipboard", uploadJson.GetAllocator());
    } else {
      return makeError(
        ERR_INVALID_REQUEST,
        "Invalid request, contentStatus.type is not 'File' or 'Clipboard'");
    }
    uploadJson.AddMember("mime",
                         rapidjson::StringRef(contentStatus.mime.data(),
                                              contentStatus.mime.size()),
                         uploadJson.GetAllocator());
    uploadJson.AddMember("hash",
                         rapidjson::StringRef(hash.data(), hash.size()),
                         uploadJson.GetAllocator());
    auto response = fetch->request(internal::HttpMethod::Put,
                                   "/room/" + std::to_string(id) + "/status",
                                   uploadJson);
    if (!response) {
      return error(response.err());
    }
    if (auto err = checkStatusCode(response.get())) {
      return err.value();
    }
    return ok();
  }
  std::optional<error_t<ErrorResponse>> ApiBridge::checkStatusCode(
    const internal::FetchResponse& response) {
    if (100 <= response.statusCode && response.statusCode < 300)
      return std::nullopt;
    if (!std::holds_alternative<rapidjson::Document>(response.body)) {
      std::string body = "";
      body.resize(std::get<std::vector<std::uint8_t>>(response.body).size());
      std::copy(std::get<std::vector<std::uint8_t>>(response.body).begin(),
                std::get<std::vector<std::uint8_t>>(response.body).end(),
                body.begin());
      std::string headers;
      for (auto itr = response.header.begin(); itr != response.header.end();
           itr++) {
        headers.append(itr->first);
        headers.append(": ");
        headers.append(itr->second);
        headers.append(" ");
      }

      return makeError(ERR_INVALID_RESPONSE,
                       "Invalid response, json not returned." + headers
                         + "status line = " + response.statusLine
                         + " body = " + body);
    }
    const rapidjson::Document& json
      = std::get<rapidjson::Document>(response.body);
    if (auto err = verifyJson(json, SCHEMA_ERROR_RESPONSE)) {
      return error(err.value());
    }
    return makeError(json["code"].GetString(), json["reason"].GetString());
  }
} // namespace octane::internal