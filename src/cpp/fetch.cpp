#include "include/fetch.h"

#include <curl/curl.h>

#include <cassert>
#include <format>

#include "include/error_code.h"
namespace octane::internal {
  Fetch::Fetch(std::string_view token,
               std::string_view origin,
               std::string_view baseUrl)
    : token(token), origin(origin), baseUrl(baseUrl) {}
  Fetch::~Fetch() {
    curl_global_cleanup();
  }
  Result<_, ErrorResponse> Fetch::init() {
    CURLcode code = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (code != 0) {
      return error(ErrorResponse{
        .code   = ERR_CURL_INITIALIZATION_FAILED,
        .reason = curl_easy_strerror(code),
      });
    }
  }
  Result<std::variant<rapidjson::Document, std::vector<std::uint8_t>>,
         ErrorResponse>
  Fetch::request(HttpMethod method, std::string_view url) {
    return request(method, url, { { "X-Octane-API-Token", token } }, {});
  }
  Result<std::variant<rapidjson::Document, std::vector<std::uint8_t>>,
         ErrorResponse>
  Fetch::request(HttpMethod method,
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
  Result<std::variant<rapidjson::Document, std::vector<std::uint8_t>>,
         ErrorResponse>
  Fetch::request(HttpMethod method,
                 std::string_view url,
                 std::string_view mimeType,
                 const std::vector<std::uint8_t>& body) {
    return request(method,
                   url,
                   { { "X-Octane-API-Token", token },
                     { "Content-Type", std::string(mimeType) } },
                   body);
  }
  Result<std::variant<rapidjson::Document, std::vector<std::uint8_t>>,
         ErrorResponse>
  Fetch::request(HttpMethod method,
                 std::string_view url,
                 const std::map<std::string, std::string>& headers,
                 const std::vector<std::uint8_t>& body) {
    auto curl = curl_easy_init();
    if (curl == nullptr) {
      return error(ErrorResponse{
        .code   = ERR_CURL_INITIALIZATION_FAILED,
        .reason = "curl is nullptr",
      });
    }

    curl_slist* list = nullptr;
    for (const auto& [key, value] : headers) {
      list = curl_slist_append(list, (key + ": " + value).c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

    switch (method) {
      case HttpMethod::Get:
        // Do nothing
        break;
      case HttpMethod::Post:
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.data());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body.size());
        break;
      case HttpMethod::Put: {
        std::pair<const std::vector<uint8_t>*, size_t> pair(&body, 0);
        curl_off_t size = body.size();
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_READDATA, &pair);
        curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, size);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, readCallback);
        break;
      }
      case HttpMethod::Delete:
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        break;
      default:
        curl_easy_cleanup(curl);
        return error(ErrorResponse{ .code   = ERR_INCORRECT_HTTP_METHOD,
                                    .reason = "incorrect http method" });
    }

    std::map<std::string, std::string> returnHeaders;
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &returnHeaders);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerCallback);

    std::vector<std::uint8_t> chunk;
    // origin = http://localhost:3000
    // baseUrl = /api/v1
    // url = /health
    // url = /room/123
    std::string uri = origin + baseUrl + std::string(url);
    curl_easy_setopt(curl, CURLOPT_URL, uri.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);

    auto code = curl_easy_perform(curl); // curlをここでしている
    curl_slist_free_all(list);
    curl_easy_cleanup(curl);

    if (code != CURLE_OK) {
      return error(ErrorResponse{ .code   = ERR_CURL_CONNECTION_FAILED,
                                  .reason = "curl connection failed" });
    }
    // curlして返ってきた結果のHTTPヘッダにContent-Type:application/jsonがあるときにはjsonを返す
    if (returnHeaders["Content-Type"].starts_with("application/json")) {
      rapidjson::Document chunkJson;
      chunkJson.Parse((char*)chunk.data());
      if (chunkJson.HasParseError())
        return error(ErrorResponse{
          .code   = ERR_JSON_PARSE_FAILED,
          .reason = "json parse failed",
        });
      return ok(chunkJson);
    }
    //そうでない時にはバイナリを返す
    return ok(chunk);
  }
  size_t Fetch::writeCallback(char* buffer,
                              size_t size,
                              size_t nmemb,
                              std::vector<std::uint8_t>* chunk) {
    chunk->reserve(chunk->size() + size * nmemb);
    for (size_t i = 0; i < size * nmemb; ++i) {
      chunk->push_back(buffer[i]);
    }
    return size * nmemb;
  }
  size_t Fetch::readCallback(
    char* buffer,
    size_t size,
    size_t nmemb,
    std::pair<const std::vector<uint8_t>*, size_t>* stream) {
    size_t len = std::min(stream->first->size() - stream->second, size * nmemb);
    memcpy(buffer, stream->first->data() + stream->second, len);
    stream->second += len;
    return len;
  }
  size_t Fetch::headerCallback(
    char* buffer,
    size_t size,
    size_t nmemb,
    std::map<std::string, std::string>* returnHeaders) {
    std::string_view buf(buffer, size * nmemb);
    auto pos = buf.find(": ");
    std::string key(buf.substr(0, pos));
    std::string val(buf.substr(pos + 2));
    (*returnHeaders)[key] = val;
    return size * nmemb;
  }
} // namespace octane::internal