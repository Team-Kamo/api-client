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
#include <regex>

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
    return request(method,
                   origin,
                   baseUrl + std::string(url),
                   { { "X-Octane-API-Token", token } },
                   {});
  }
  Fetch::FetchResult Fetch::request(HttpMethod method,
                                    std::string_view url,
                                    const rapidjson::Document& body) {
    if (method != HttpMethod::Post && method != HttpMethod::Put) {
      return makeError(
        ERR_INCORRECT_HTTP_METHOD,
        "Only Post and Put requests are allowed for requests with a body parts.");
    }
    std::vector<uint8_t> decoded;
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    body.Accept(writer);

    decoded.resize(buffer.GetSize());
    std::copy(buffer.GetString(),
              buffer.GetString() + buffer.GetSize(),
              decoded.begin());

    return request(method,
                   origin,
                   baseUrl + std::string(url),
                   { { "X-Octane-API-Token", token },
                     { "Content-Type", "application/json" } },
                   decoded);
  }
  Fetch::FetchResult Fetch::request(HttpMethod method,
                                    std::string_view url,
                                    std::string_view mimeType,
                                    const std::vector<std::uint8_t>& body) {
    if (method != HttpMethod::Post && method != HttpMethod::Put) {
      return makeError(
        ERR_INCORRECT_HTTP_METHOD,
        "Only Post and Put requests are allowed for requests with a body parts.");
    }
    return request(method,
                   origin,
                   baseUrl + std::string(url),
                   { { "X-Octane-API-Token", token },
                     { "Content-Type", std::string(mimeType) } },
                   body);
  }
  Fetch::FetchResult Fetch::request(
    HttpMethod method,
    std::string_view origin,
    std::string_view url,
    const std::map<std::string, std::string>& headers,
    const std::vector<std::uint8_t>& body) {
    HttpRequest request{
      .method      = method,
      .version     = HttpVersion::Http2,
      .uri         = std::string(url),
      .headerField = headers,
      .body        = body,
    };
    auto result = client->request(origin, request);
    if (!result) {
      return error(result.err());
    }

    auto& response = result.get();
    if (300 <= response.statusCode && response.statusCode < 400) {
      const auto& location = response.headerField["Location"];
      std::smatch regexResults;
      if (std::regex_search(
            location, regexResults, std::regex(R"(^(https?://.+)(/.*)?)"))) {
        auto origin = regexResults[1].str();
        auto url    = regexResults[2].str();
        if (url.empty()) url = "/";
        return this->request(method, origin, url, headers, body);
      }
    }

    FetchResponse fetchResponse;
    fetchResponse.statusLine = response.statusLine;
    fetchResponse.statusCode = response.statusCode;
    std::string contentType  = response.headerField["Content-Type"];
    auto pos                 = contentType.find("; ");
    fetchResponse.mime       = contentType.substr(0, pos);
    // curlして返ってきた結果のHTTPヘッダにContent-Type:application/jsonがあるときにはFetchResponse.bodyにjsonを代入する
    if (fetchResponse.mime == "application/json") {
      rapidjson::Document json;
      json.Parse((char*)response.body.data());
      if (json.HasParseError()) {
        const auto offset  = json.GetErrorOffset();
        const auto message = rapidjson::GetParseError_En(json.GetParseError());
        return makeError(
          ERR_JSON_PARSE_FAILED,
          message + std::string("\noffset: ") + std::to_string(offset));
      }
      fetchResponse.body = std::move(json);
    }
    //そうでない時にはFetchResponse.bodyにバイナリを代入する
    else {
      fetchResponse.body = std::move(response.body);
    }
    return ok(fetchResponse);
  }
} // namespace octane::internal