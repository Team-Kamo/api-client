/**
 * @file fetch.h
 * @author soon (kento.soon@gmail.com)
 * @brief fetch.hの実装
 * @version 0.1
 * @date 2022-09-01
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "include/internal/fetch.h"

#include <rapidjson/error/en.h>

#include <cassert>
#include <format>

#include "include/error_code.h"

namespace octane::internal {
  Fetch::Fetch(std::string_view token,
               std::string_view origin,
               std::string_view baseUrl,
               HttpClientBase* client)
    : token(token), origin(origin), baseUrl(baseUrl), client(client) {}
  Result<_, ErrorResponse> Fetch::init() {
    return client->init();
  }

  Fetch::FetchResult Fetch::request(HttpMethod method, std::string_view url) {
    return request(method, url, { { "X-Octane-API-Token", token } }, {});
  }
  Fetch::FetchResult Fetch::request(HttpMethod method,
                                    std::string_view url,
                                    const rapidjson::Document& body) {
    std::vector<uint8_t> decoded;
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    body.Accept(writer);

    decoded.resize(buffer.GetSize());
    std::copy(buffer.GetString(),
              buffer.GetString() + buffer.GetSize(),
              decoded.begin());

    return request(method,
                   url,
                   { { "X-Octane-API-Token", token },
                     { "Content-Type", "application/json" } },
                   decoded);
  }
  Fetch::FetchResult Fetch::request(HttpMethod method,
                                    std::string_view url,
                                    std::string_view mimeType,
                                    const std::vector<std::uint8_t>& body) {
    return request(method,
                   url,
                   { { "X-Octane-API-Token", token },
                     { "Content-Type", std::string(mimeType) } },
                   body);
  }
  Fetch::FetchResult Fetch::request(
    HttpMethod method,
    std::string_view url,
    const std::map<std::string, std::string>& headers,
    const std::vector<std::uint8_t>& body) {
    HttpRequest request{
      .method      = method,
      .version     = HttpVersion::Http2,
      .uri         = baseUrl + std::string(url),
      .headerField = headers,
      .body        = body,
    };
    auto result = client->request(origin, request);
    if (!result) {
      return error(result.err());
    }

    auto& response = result.get();

    // curlして返ってきた結果のHTTPヘッダにContent-Type:application/jsonがあるときにはjsonを返す
    if (response.headerField["Content-Type"].starts_with("application/json")) {
      rapidjson::Document json;
      json.Parse((char*)response.body.data());
      if (json.HasParseError()) {
        const auto offset  = json.GetErrorOffset();
        const auto message = rapidjson::GetParseError_En(json.GetParseError());
        return makeError(
          ERR_JSON_PARSE_FAILED,
          message + std::string("\noffset: ") + std::to_string(offset));
      }
      return ok(json);
    }
    //そうでない時にはバイナリを返す
    return ok(response.body);
  }
} // namespace octane::internal