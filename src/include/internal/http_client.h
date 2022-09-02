/**
 * @file http_client.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief HTTPクライアント。
 * @version 0.1
 * @date 2022-08-31
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef OCTANE_API_CLIENT_INTERNAL_HTTP_CLIENT_H_
#define OCTANE_API_CLIENT_INTERNAL_HTTP_CLIENT_H_

#include <gtest/gtest_prod.h>

#include <map>
#include <string>
#include <vector>

#include "../error_response.h"
#include "../result.h"

namespace octane::internal {
  /**
   * @brief HTTPメソッドを表す列挙体。
   *
   */
  enum struct HttpMethod {
    /** @brief GETメソッドを表す。*/
    Get,
    /** @brief POSTメソッドを表す。*/
    Post,
    /** @brief PUTメソッドを表す。*/
    Put,
    /** @brief DELETEメソッドを表す。*/
    Delete,
  };
  /**
   * @brief HTTPバージョンを表す。
   *
   */
  enum struct HttpVersion {
    /** @brief HTTP/1.0を表す。*/
    Http1_0,
    /** @brief HTTP/1.1を表す。*/
    Http1_1,
    /** @brief HTTP/2を表す。*/
    Http2,
    /** @brief HTTP/3を表す。*/
    Http3,
  };
  /**
   * @brief HTTPのリクエストを表す構造体。
   *
   */
  struct HttpRequest {
    /** @brief リクエストに使用するHTTPメソッド。*/
    HttpMethod method;
    /** @brief リクエストに使用するHTTPバージョン。*/
    HttpVersion version;
    /** @brief リクエスト先のURI。*/
    std::string uri;
    /** @brief リクエストに使用するHTTPヘッダフィールド。*/
    std::map<std::string, std::string> headerField;
    /** @brief リクエストのボディ部。*/
    std::vector<std::uint8_t> body;
  };
  bool operator==(const HttpRequest& a, const HttpRequest& b);
  std::ostream& operator<<(std::ostream& stream, const HttpRequest& request);

  /**
   * @brief HTTPのレスポンスを表す構造体。
   *
   */
  struct HttpResponse {
    /**
     * @brief レスポンスのHTTPステータスコードを表す。
     * @details
     * - 1xx: 情報レスポンス
     * - 2xx: 成功レスポンス
     * - 3xx: リダイレクトメッセージ
     * - 4xx: クライアントエラーレスポンス
     * - 5xx: サーバエラーレスポンス
     *
     * @see { @link https://developer.mozilla.org/ja/docs/Web/HTTP/Status }
     *
     */
    int statusCode;
    /**
     * @brief レスポンスのHTTPステータスライン。"HTTP/2 200 OK", "HTTP/2 400 Bad
     * Request"など。
     *
     */
    std::string statusLine;
    /**
     * @brief レスポンスのHTTPのバージョン。
     *
     */
    HttpVersion version;
    /**
     * @brief レスポンスのヘッダフィールド。
     *
     */
    std::map<std::string, std::string> headerField;
    /**
     * @brief レスポンスのボディ部。
     *
     */
    std::vector<std::uint8_t> body;
  };
  bool operator==(const HttpResponse& a, const HttpResponse& b);
  std::ostream& operator<<(std::ostream& stream, const HttpResponse& response);

  /**
   * @brief HTTP通信を行うインタフェース。
   *
   * @see HttpClient このインタフェースを実装したクラス。
   *
   */
  class HttpClientBase {
  public:
    virtual ~HttpClientBase() noexcept = 0;

    /**
     * @brief HttpClientを初期化する。
     *
     * @details
     * このメソッドはインスタンスを生成した直後に呼び出さなければならない。
     * また、複数回呼び出すことはできない。
     * 失敗した場合は以下のエラーレスポンスを返す。
     * - ERR_CURL_INITIALIZATION_FAILED: CURLの初期化に失敗したとき
     *
     * @return Result<_, ErrorResponse>
     * 成功した場合は何も返さず、失敗した場合は上記のエラーレスポンスを返す。
     */
    virtual Result<_, ErrorResponse> init() noexcept = 0;

    /**
     * @brief HTTPリクエストを発行する。
     * @details
     * このメソッドは事前に一度必ず{ @see init }を呼ばなければならない。
     * 失敗した場合は以下のエラーレスポンスを返す。
     * - ERR_INCORRECT_HTTP_METHOD: GET, POST, PUT,
     * DELETE以外のメソッドを使用したり、GET, DELETEでボディ部を指定したとき
     * - ERR_CURL_CONNECTION_FAILED: CURLの接続に失敗したとき
     *
     * @param[in] origin リクエスト先のオリジン。"http://localhost:3000"など。
     * @param[in] request リクエスト用のオブジェクト。
     * @return Result<HttpResponse, ErrorResponse>
     */
    virtual Result<HttpResponse, ErrorResponse>
    request(std::string_view origin, const HttpRequest& request) = 0;
  };
  /**
   * @brief HTTP通信を行う。
   *
   */
  class HttpClient : public HttpClientBase {
    FRIEND_TEST(HttpClientTest, WriteCallback);
    FRIEND_TEST(HttpClientTest, ReadCallback);
    FRIEND_TEST(HttpClientTest, HeaderCallback);

  public:
    virtual ~HttpClient() noexcept;

    /**
     * {@inheritDoc}
     */
    virtual Result<_, ErrorResponse> init() noexcept override;
    /**
     * {@inheritDoc}
     */
    virtual Result<HttpResponse, ErrorResponse> request(
      std::string_view origin,
      const HttpRequest& request) override;

  private:
    /**
     * @brief CURLでレスポンスのボディ部を受け取るためのコールバック。
     *
     * @see { @link https://curl.se/libcurl/c/CURLOPT_WRITEFUNCTION.html }
     *
     * @param[in] buffer ボディ部の一部が格納されているバッファ。
     * @param[in] size 常に1。
     * @param[in] nmemb バッファのサイズ。
     * @param[in,out] chunk 受け取るための領域。
     * @return size_t 処理したバッファのバイト数。
     */
    static size_t writeCallback(char* buffer,
                                size_t size,
                                size_t nmemb,
                                std::vector<std::uint8_t>* chunk);

    /**
     * @brief CURLでリクエストのボディ部に書き込むためのコールバック。
     *
     * @see { @link https://curl.se/libcurl/c/CURLOPT_READFUNCTION.html }
     *
     * @param[in,out] buffer 書き込み先のバッファ。
     * @param[in] size これとnmembを掛けた値がバッファのサイズ。
     * @param[in] nmemb これとsizeを掛けた値がバッファのサイズ。
     * @param[in] stream 書き込み元のバッファ。
     * @return size_t 書き込んだバッファのバイト数。
     */
    static size_t readCallback(
      char* buffer,
      size_t size,
      size_t nmemb,
      std::pair<const std::vector<uint8_t>*, size_t>* stream);

    /**
     * @brief CURLでレスポンスのヘッダ部を受け取るためのコールバック。
     * ヘッダの一行ごとに呼ばれる。
     *
     * @see { @link https://curl.se/libcurl/c/CURLOPT_HEADERFUNCTION.html }
     *
     * @param[in] buffer 完全な一行を含む文字列バッファ。
     * @param[in] size 常に1。
     * @param[in] nmemb バッファのバイト数。
     * @param[in,out] responseHeaderField 受け取るための領域。
     * @return size_t 処理したバイト数。
     */
    static size_t headerCallback(
      char* buffer,
      size_t size,
      size_t nmemb,
      std::pair<std::string, std::map<std::string, std::string>>*
        responseHeader);
  };
} // namespace octane::internal

#endif // OCTANE_API_CLIENT_HTTP_CLIENT_H_