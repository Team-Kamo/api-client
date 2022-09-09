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

#include <algorithm>
#include <cstring>
#include <ostream>

#include "include/error_code.h"

namespace octane::internal {
  namespace {
    struct data {
      char trace_ascii; /* 1 or 0 */
    };

    void dump(const char* text,
              FILE* stream,
              unsigned char* ptr,
              size_t size,
              char nohex) {
      size_t i;
      size_t c;

      unsigned int width = 0x10;

      if (nohex) /* without the hex output, we can fit more on screen */
        width = 0x40;

      fprintf(stream,
              "%s, %10.10lu bytes (0x%8.8lx)\n",
              text,
              (unsigned long)size,
              (unsigned long)size);

      for (i = 0; i < size; i += width) {
        fprintf(stream, "%4.4lx: ", (unsigned long)i);

        if (!nohex) {
          /* hex not disabled, show it */
          for (c = 0; c < width; c++)
            if (i + c < size)
              fprintf(stream, "%02x ", ptr[i + c]);
            else
              fputs("   ", stream);
        }

        for (c = 0; (c < width) && (i + c < size); c++) {
          /* check for 0D0A; if found, skip past and start a new line of output
           */
          if (nohex && (i + c + 1 < size) && ptr[i + c] == 0x0D
              && ptr[i + c + 1] == 0x0A) {
            i += (c + 2 - width);
            break;
          }
          fprintf(
            stream,
            "%c",
            (ptr[i + c] >= 0x20) && (ptr[i + c] < 0x80) ? ptr[i + c] : '.');
          /* check again for 0D0A, to avoid an extra \n if it's at width */
          if (nohex && (i + c + 2 < size) && ptr[i + c + 1] == 0x0D
              && ptr[i + c + 2] == 0x0A) {
            i += (c + 3 - width);
            break;
          }
        }
        fputc('\n', stream); /* newline */
      }
      fflush(stream);
    }

    int trace(CURL* handle,
              curl_infotype type,
              char* data,
              size_t size,
              void* userp) {
      struct data* config = (struct data*)userp;
      const char* text;
      (void)handle; /* prevent compiler warning */

      switch (type) {
        case CURLINFO_TEXT:
          fprintf(stderr, "== Info: %s", data);
          /* FALLTHROUGH */
        default: /* in case a new one is introduced to shock us */
          return 0;

        case CURLINFO_HEADER_OUT:
          text = "=> Send header";
          break;
        case CURLINFO_DATA_OUT:
          text = "=> Send data";
          break;
        case CURLINFO_SSL_DATA_OUT:
          text = "=> Send SSL data";
          break;
        case CURLINFO_HEADER_IN:
          text = "<= Recv header";
          break;
        case CURLINFO_DATA_IN:
          text = "<= Recv data";
          break;
        case CURLINFO_SSL_DATA_IN:
          text = "<= Recv SSL data";
          break;
      }

      dump(text, stderr, (unsigned char*)data, size, config->trace_ascii);
      return 0;
    }
  } // namespace
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

#ifndef NDEBUG
    // struct data config;
    // config.trace_ascii = 1;
    // curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, trace);
    // curl_easy_setopt(curl, CURLOPT_DEBUGDATA, &config);
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    // curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
#endif // #ifndef NDEBUG

    // HTTPヘッダを定義する。
    curl_slist* list = nullptr;
    for (const auto& [key, value] : request.headerField) {
      list = curl_slist_append(list, (key + ": " + value).c_str());
    }
    list = curl_slist_append(list, "Expect:");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

    // PUTメソッド用
    std::pair<const std::vector<std::uint8_t>*, size_t> pair(request.body, 0);

    // HTTPメソッドごとに処理を分岐。
    switch (request.method) {
      case HttpMethod::Get:
        if (!request.body->empty()) {
          return makeError(ERR_INCORRECT_HTTP_METHOD,
                           "Request body must be empty.");
        }
        break;
      case HttpMethod::Post:
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.body->data());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, request.body->size());
        break;
      case HttpMethod::Put:
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_READDATA, &pair);
        curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, request.body->size());
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, readCallback);
        break;
      case HttpMethod::Delete:
        if (!request.body->empty()) {
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

    HttpResponse response;

    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerCallback);

    // レスポンスのボディを受け取るための準備
    const std::string uri = std::string(origin) + request.uri;
    curl_easy_setopt(curl, CURLOPT_URL, uri.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // 通信を開始する。
    const CURLcode code = curl_easy_perform(curl);

    // 終了処理。
    curl_slist_free_all(list);
    curl_easy_cleanup(curl);

    if (code != CURLE_OK) {
      return makeError(ERR_CURL_CONNECTION_FAILED, curl_easy_strerror(code));
    }
    //レスポンスを正しい形にして返す。
    return makeHttpResponse(std::move(response));
  }

  size_t HttpClient::writeCallback(char* buffer,
                                   size_t size,
                                   size_t nmemb,
                                   HttpResponse* response) {
    std::copy_n(buffer, nmemb, std::back_inserter(response->body));
    return size * nmemb;
  }

  size_t HttpClient::readCallback(
    char* buffer,
    size_t size,
    size_t nmemb,
    std::pair<const std::vector<std::uint8_t>*, size_t>* stream) {
    size_t len = std::min(stream->first->size() - stream->second, size * nmemb);
    std::copy_n(stream->first->data() + stream->second, len, buffer);
    stream->second += len;
    return len;
  }

  size_t HttpClient::headerCallback(char* buffer,
                                    size_t size,
                                    size_t nmemb,
                                    HttpResponse* response) {
    // 扱いやすいようにstring_viewでラップ(コピーが発生しないのでオーバーヘッドは無視できるほど小さいはず多分)
    std::string_view buf(buffer, size * nmemb);
    if (response->statusLine.empty()) {
      response->statusLine = buf.substr(0, buf.size() - 2);
    } else {
      auto pos = buf.find(": ");
      std::string key(buf.substr(0, pos));
      buf = buf.substr(pos + 2);
      std::string val(buf.substr(0, buf.size() - 2));
      response->headerField[key] = val;
      if (key == "Content-Length") {
        response->body.reserve(std::stoll(val));
      }
    }
    return size * nmemb;
  }
  Result<HttpResponse, ErrorResponse> HttpClient::makeHttpResponse(
    HttpResponse&& response) {
    auto pos1           = response.statusLine.find(" ");
    auto pos2           = response.statusLine.substr(pos1 + 1).find(" ");
    response.statusCode = std::stoi(response.statusLine.substr(pos1 + 1, pos2));
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
      return makeError(ERR_INVALID_RESPONSE, "http version was invalid");
    }
    return ok(std::move(response));
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
    if (a.version != b.version) return false;
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
    body.resize(request.body->size());
    std::copy(request.body->begin(), request.body->end(), body.begin());

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

    std::string version;
    switch (response.version) {
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

    stream << "statusLine = " << response.statusLine
           << ", statusCode = " << response.statusCode
           << ", version = " << version << ", headers = " << headers
           << ", body = " << body;

    return stream;
  }
} // namespace octane::internal