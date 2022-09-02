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

#include <cstring>
#include <ostream>

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
        std::pair<const std::vector<std::uint8_t>*, size_t> pair(&request.body,
                                                                 0);
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
    std::pair<std::string, std::map<std::string, std::string>> responseHeader;
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &responseHeader);
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
    response.statusLine     = std::move(responseHeader.first);
    auto pos1               = response.statusLine.find(" ");
    auto pos2               = response.statusLine.substr(pos1).find(" ");
    response.statusCode     = stoi(response.statusLine.substr(pos1, pos2));
    std::string httpVersion = response.statusLine.substr(0, pos1);
    if (httpVersion == "HTTP/1.0") {
      response.version = HttpVersion::Http1_0;
    } else if (httpVersion == "HTTP/1.1") {
      response.version = HttpVersion::Http1_1;
    } else if (httpVersion == "HTTP/2") {
      response.version = HttpVersion::Http2;
    } else if (httpVersion == "HTTP/3") {
      response.version = HttpVersion::Http3;
    } else {
      makeError(ERR_INVALID_RESPONSE, "http version was invalid");
    }
    response.headerField = std::move(responseHeader.second);
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
    std::pair<const std::vector<std::uint8_t>*, size_t>* stream) {
    size_t len = std::min(stream->first->size() - stream->second, size * nmemb);
    std::memcpy(buffer, stream->first->data() + stream->second, len);
    stream->second += len;
    return len;
  }

  size_t HttpClient::headerCallback(
    char* buffer,
    size_t size,
    size_t nmemb,
    std::pair<std::string, std::map<std::string, std::string>>*
      responseHeader) {
    // 扱いやすいようにstring_viewでラップ(コピーが発生しないのでオーバーヘッドは無視できるほど小さいはず多分)
    std::string_view buf(buffer, size * nmemb);
    auto pos = buf.find(": ");
    if (pos == buf.npos) {
      responseHeader->first = buf;
    } else {
      std::string key(buf.substr(0, pos));
      std::string val(buf.substr(pos + 2));
      responseHeader->second[key] = val;
    }
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
  std::ostream& operator<<(std::ostream& stream, const HttpRequest& request) {
    std::string method;
    switch (request.method) {
      case HttpMethod::Get:
        method = "GET";
        break;
      case HttpMethod::Post:
        method = "POST";
        break;
      case HttpMethod::Put:
        method = "PUT";
        break;
      case HttpMethod::Delete:
        method = "DELETE";
        break;
      default:
        method = "UNKNOWN";
        break;
    }

    std::string version;
    switch (request.version) {
      case HttpVersion::Http1_0:
        version = "HTTP/1.0";
        break;
      case HttpVersion::Http1_1:
        version = "HTTP/1.1";
        break;
      case HttpVersion::Http2:
        version = "HTTP/2";
        break;
      case HttpVersion::Http3:
        version = "HTTP/3";
        break;
      default:
        version = "UNKNOWN";
    }

    std::string headers;
    headers += "{ ";
    for (const auto& [key, value] : request.headerField) {
      headers += key;
      headers += ": ";
      headers += value;
      headers += ", ";
    }
    headers += "}";

    std::string body;
    body.resize(request.body.size());
    std::copy(request.body.begin(), request.body.end(), body.begin());

    stream << "method = " << method << ", version = " << version
           << ", uri = " << request.uri << ", headers = " << headers
           << ", body = " << body;
    return stream;
  }
  std::ostream& operator<<(std::ostream& stream, const HttpResponse& response) {
    std::string headers;
    headers += "{ ";
    for (const auto& [key, value] : response.headerField) {
      headers += key;
      headers += ": ";
      headers += value;
      headers += ", ";
    }
    headers += "}";

    std::string body;
    body.resize(response.body.size());
    std::copy(response.body.begin(), response.body.end(), body.begin());

    stream << "statusLine = " << response.statusLine
           << ", headers = " << headers << ", body = " << body;

    return stream;
  }
} // namespace octane::internal