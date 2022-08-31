#include "include/fetch.h"

#include <curl/curl.h>

#include "include/error_code.h"
namespace octane::internal {
  Fetch::Fetch(std::string_view token, std::string_view origin)
    : token(token), origin(origin) {}
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
  request(HttpMethod method, std::string_view url) {
    auto curl = curl_easy_init();
    if (curl == nullptr) {
      return error(ErrorResponse{
        .code   = ERR_CURL_INITIALIZATION_FAILED,
        .reason = "curl is nullptr",
      });
    }
    std::string chunk;
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);
    switch (method) {
      case HttpMethod::Get:
        break;
      case HttpMethod::Delete:
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        break;
      default:
        curl_easy_cleanup(curl);
        return error(ErrorResponse{ .code   = ERR_INCORRECT_HTTP_METHOD,
                                    .reason = "incorrect http method" });
    }
    return error(ErrorResponse{});
  }
  Result<std::variant<rapidjson::Document, std::vector<std::uint8_t>>,
         ErrorResponse>
  request(HttpMethod method,
          std::string_view url,
          const rapidjson::Document& body) {
    auto curl = curl_easy_init();
    if (curl == nullptr) {
      return error(ErrorResponse{
        .code   = ERR_CURL_INITIALIZATION_FAILED,
        .reason = "curl is nullptr",
      });
    }
    std::string chunk;
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);
    switch (method) {
      case HttpMethod::Get:
        break;
      case HttpMethod::Post:
        break;
      case HttpMethod::Put:
        break;
      case HttpMethod::Delete:
        break;
      default:
        curl_easy_cleanup(curl);
        return error(ErrorResponse{ .code   = ERR_INCORRECT_HTTP_METHOD,
                                    .reason = "incorrect http method" });
    }
    return error(ErrorResponse{});
  }
  Result<std::variant<rapidjson::Document, std::vector<std::uint8_t>>,
         ErrorResponse>
  request(HttpMethod method,
          std::string_view url,
          const std::vector<std::uint8_t>& body) {
    auto curl = curl_easy_init();
    if (curl == nullptr) {
      return error(ErrorResponse{
        .code   = ERR_CURL_INITIALIZATION_FAILED,
        .reason = "curl is nullptr",
      });
    }
    switch (method) {
      case HttpMethod::Get:
        break;
      case HttpMethod::Post:
        break;
      case HttpMethod::Put:
        break;
      case HttpMethod::Delete:
        break;
      default:
        curl_easy_cleanup(curl);
        return error(ErrorResponse{ .code   = ERR_INCORRECT_HTTP_METHOD,
                                    .reason = "incorrect http method" });
    }
    return error(ErrorResponse{});
  }
} // namespace octane::internal