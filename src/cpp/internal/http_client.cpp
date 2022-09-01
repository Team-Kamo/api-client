/**
 * @file http_client.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief http_client.hの実装。
 * @version 0.1
 * @date 2022-08-31
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "include/internal/http_client.h"

// curlってwinapiインクルードしちゃってるのね。
#define NOMINMAX

#include <curl/curl.h>

#include "include/error_code.h"

namespace octane::internal {
  HttpClientBase::~HttpClientBase() {}
  HttpClient::~HttpClient() {
    curl_global_cleanup();
  }
  Result<_, ErrorResponse> HttpClient::init() noexcept {
    const CURLcode code = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (code != 0) {
      return makeError(ERR_CURL_INITIALIZATION_FAILED,
                       curl_easy_strerror(code));
    }
    return ok();
  }

  Result<HttpResponse, ErrorResponse> HttpClient::request(
    std::string_view origin,
    const HttpRequest& request) {
    // CURLの初期化。ふつうは失敗しないはず。
    const auto curl = curl_easy_init();
    if (curl == nullptr) {
      return makeError(ERR_CURL_INITIALIZATION_FAILED, "curl is nullptr");
    }

    // HTTPヘッダを定義する。
    curl_slist* list = nullptr;
    for (const auto& [key, value] : request.headerField) {
      list = curl_slist_append(list, (key + ": " + value).c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

    // HTTPメソッドごとに処理を分岐。
    switch (request.method) {
      case HttpMethod::Get:
        if (!request.body.empty()) {
          return makeError(ERR_INCORRECT_HTTP_METHOD,
                           "Request body must be empty.");
        }
        break;
      case HttpMethod::Post:
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.body.data());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, request.body.size());
        break;
      case HttpMethod::Put: {
        std::pair<const std::vector<uint8_t>*, size_t> pair(&request.body, 0);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_READDATA, &pair);
        curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, request.body.size());
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, readCallback);
        break;
      }
      case HttpMethod::Delete:
        if (!request.body.empty()) {
          return makeError(ERR_INCORRECT_HTTP_METHOD,
                           "Request body must be empty.");
        }
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        break;
      default:
        curl_easy_cleanup(curl);
        return makeError(
          ERR_INCORRECT_HTTP_METHOD,
          "An undefined method was specified. Available methods are GET, POST, PUT, and DELETE.");
    }

    // レスポンスのヘッダを受け取るための準備
    std::map<std::string, std::string> responseHeaderField;
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &responseHeaderField);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerCallback);

    // レスポンスのボディを受け取るための準備
    std::vector<std::uint8_t> chunk;
    const std::string uri = std::string(origin) + request.uri;
    curl_easy_setopt(curl, CURLOPT_URL, uri.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);

    // 通信を開始する。
    const CURLcode code = curl_easy_perform(curl);

    // 終了処理。
    curl_slist_free_all(list);
    curl_easy_cleanup(curl);

    if (code != CURLE_OK) {
      return makeError(ERR_CURL_CONNECTION_FAILED, curl_easy_strerror(code));
    }

    HttpResponse response;
    response.statusLine  = "";
    response.headerField = std::move(responseHeaderField);
    response.body        = std::move(chunk);

    return ok(response);
  }

  size_t HttpClient::writeCallback(char* buffer,
                                   size_t size,
                                   size_t nmemb,
                                   std::vector<std::uint8_t>* chunk) {
    chunk->reserve(chunk->size() + size * nmemb);
    for (size_t i = 0; i < size * nmemb; ++i) {
      chunk->push_back(buffer[i]);
    }
    return size * nmemb;
  }

  size_t HttpClient::readCallback(
    char* buffer,
    size_t size,
    size_t nmemb,
    std::pair<const std::vector<uint8_t>*, size_t>* stream) {
    size_t len = std::min(stream->first->size() - stream->second, size * nmemb);
    memcpy(buffer, stream->first->data() + stream->second, len);
    stream->second += len;
    return len;
  }

  size_t HttpClient::headerCallback(
    char* buffer,
    size_t size,
    size_t nmemb,
    std::map<std::string, std::string>* responseHeaderField) {
    // 扱いやすいようにstring_viewでラップ(コピーが発生しないのでオーバーヘッドは無視できるほど小さいはず多分)
    std::string_view buf(buffer, size * nmemb);
    auto pos = buf.find(": ");
    std::string key(buf.substr(0, pos));
    std::string val(buf.substr(pos + 2));
    (*responseHeaderField)[key] = val;
    return size * nmemb;
  }

  bool operator==(const HttpRequest& a, const HttpRequest& b) {
    if (a.method != b.method) return false;
    if (a.version != b.version) return false;
    if (a.uri != b.uri) return false;
    if (a.headerField != b.headerField) return false;
    if (a.body != b.body) return false;
    return true;
  }
  bool operator==(const HttpResponse& a, const HttpResponse& b) {
    if (a.statusCode != b.statusCode) return false;
    if (a.statusLine != b.statusLine) return false;
    if (a.statusCode != b.statusCode) return false;
    if (a.headerField != b.headerField) return false;
    if (a.body != b.body) return false;
    return true;
  }
} // namespace octane::internal