/**
 * @file fetch.h
 * @author soon (kento.soon@gmail.com)
 * @brief Fetchクラスを定義する。
 * @version 0.1
 * @date 2022-09-01
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef OCTANE_API_CLIENT_INTERNAL_FETCH_H_
#define OCTANE_API_CLIENT_INTERNAL_FETCH_H_

#include <rapidjson/document.h>
#include <rapidjson/encodings.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <map>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "./http_client.h"
#include "include/error_response.h"
#include "include/result.h"

namespace octane::internal {
  /**
   * @brief Fetchのレスポンスを表す構造体。
   *
   */
  struct FetchResponse {
    /** @brief レスポンスのボディ部。*/
    std::variant<rapidjson::Document, std::vector<std::uint8_t>> body;
    /** @brief レスポンスのmime。*/
    std::string mime;
    /** @brief レスポンスのステータスコード。*/
    int statusCode;
    /** @brief レスポンスのステータスライン。 */
    std::string statusLine;
  };

  /**
   * @brief HttpClientクラスを通じてHTTP通信を行うインタフェース。
   * @details
   * このインタフェースを実装したクラスは3xx番台のレスポンスを受けたときにリダイレクト処理を行わなければならない。
   * また、このリダイレクト処理はHTTPヘッダのLocationに従う。
   * { @see Fetch } このインタフェースを実装したクラス。
   *
   */
  class FetchBase {
  public:
    using FetchResult             = Result<FetchResponse, ErrorResponse>;
    virtual ~FetchBase() noexcept = 0;
    /**
     * @brief Fetchのインスタンスを初期化する。
     * @details
     * このメソッドはインスタンス一つにつき一度だけ呼び出すことができる。
     * また、インスタンスを作成した直後に呼び出さなければならない。
     * 失敗した場合は次のエラーレスポンスを返す。
     * - ERR_CURL_INITIALIZATION_FAILED: CURLの初期化に失敗したとき
     *
     * @return Result<_, ErrorResponse>
     * 成功した場合は何も返さず、失敗した場合は上記のエラーレスポンスを返す。
     */
    virtual Result<_, ErrorResponse> init() = 0;
    /**
     * @brief APIへのボディ部を持たないリクエストを発行する。
     * @details
     * このメソッドはボディを持たないリクエストに使用する。
     * GET及びDELETEリクエストは必ずこのメソッドを使用しなければならない。
     * 失敗した場合は次のエラーレスポンスを返す。
     * - ERR_JSON_PARSE_FAILED:
     * レスポンスのContent-Typeがapplication/jsonであったにもかかわらず正常なJSONデータがAPIから返却されなかったとき
     * - ERR_INCORRECT_HTTP_METHOD: GET, POST, PUT,
     * DELETE以外のHTTPメソッドを指定したとき
     * - ERR_CURL_CONNECTION_FAILED: CURLの接続に失敗したとき
     *
     * @param[in] method リクエストに使用するHTTPメソッド
     * @param[in] url APIへのURL
     * @return FetchResult
     * 成功した場合はレスポンスのボディ部、失敗した場合は上記のエラーレスポンスを返す。
     */
    virtual FetchResult request(HttpMethod method, std::string_view url) = 0;
    /**
     * @brief APIへのJSON形式のボディ部を持つリクエストを発行する。
     * @details
     * このメソッドはJSON形式のボディを持つリクエストに使用する。
     * GET及びDELETEリクエストはこのメソッドを使用してはならず、代わりに{@link
     * Fetch::request(HttpMethod method, std::string_view url)}を使用すること。
     * 失敗した場合は次のエラーレスポンスを返す。
     * - ERR_JSON_PARSE_FAILED:
     * レスポンスのContent-Typeがapplication/jsonであったにもかかわらず正常なJSONデータがAPIから返却されなかったとき
     * - ERR_INCORRECT_HTTP_METHOD: POST, PUT以外のHTTPメソッドを指定したとき
     * - ERR_CURL_CONNECTION_FAILED: CURLの接続に失敗したとき
     *
     * @param[in] method リクエストに使用するHTTPメソッド
     * @param[in] url APIへのURL
     * @param[in] body APIリクエストのボディ部
     * @return FetchResult
     * 成功した場合はレスポンスのボディ部、失敗した場合は上記のエラーレスポンスを返す。
     */
    virtual FetchResult request(HttpMethod method,
                                std::string_view url,
                                const rapidjson::Document& body)
      = 0;
    /**
     * @brief APIへの任意のContent-Typeのボディ部を持つリクエストを発行する。
     * @details
     * このメソッドは任意のContent-Typeのボディを持つリクエストに使用する。
     * JSON形式のリクエストの場合は{@link Fetch::request(HttpMethod method,
     * std::string_view url, const rapidjson::Document& body)}の使用を推奨する。
     * GET及びDELETEリクエストはこのメソッドを使用してはならず、代わりに{@link
     * Fetch::request(HttpMethod method, std::string_view url)}を使用すること。
     * 失敗した場合は次のエラーレスポンスを返す。
     * - ERR_JSON_PARSE_FAILED:
     * レスポンスのContent-Typeがapplication/jsonであったにもかかわらず正常なJSONデータがAPIから返却されなかったとき
     * - ERR_INCORRECT_HTTP_METHOD: POST, PUT以外のHTTPメソッドを指定したとき
     * - ERR_CURL_CONNECTION_FAILED: CURLの接続に失敗したとき
     *
     * @param[in] method リクエストに使用するHTTPメソッド
     * @param[in] url APIへのURL
     * @param[in] body APIリクエストのボディ部
     * @return FetchResult
     * 成功した場合はレスポンスのボディ部、失敗した場合は上記のエラーレスポンスを返す。
     */
    virtual FetchResult request(HttpMethod method,
                                std::string_view url,
                                std::string_view mimeType,
                                const std::vector<std::uint8_t>& body)
      = 0;
  };
  /**
   * @brief HttpClientクラスを通じてHTTP通信を行う。
   * @details
   * このクラスは3xx番台のレスポンスを受けたときにリダイレクト処理を行う。
   * また、このリダイレクト処理はHTTPヘッダのLocationに従う。
   *
   */
  class Fetch : public FetchBase {
    std::string token;
    std::string origin;
    std::string baseUrl;
    HttpClientBase* client;

  public:
    /**
     * @brief Construct a new Fetch object
     * @details
     * tokenは正当なAPIトークンでなければならない。
     * また、このトークンはHTTP拡張ヘッダ"X-Octane-API-Token"で送信される。
     * originはAPIサーバへのプロトコル、ドメイン、ポート番号を含む正当なAPIサーバのオリジンでなければならない。
     * 例えば"http://localhost:3000"などの形式となる。
     * baseUrlはAPIと通信するときにオリジンの後につく共通のURLを指定する。
     * 例えば"/api/v1"など。
     * これは{@link Fetch::request}のurl引数のベースURLとなる。
     * clientはHTTP通信を仲介する{@link
     * HttpClientBase}を実装する任意のインスタンスを指定する。通常は{@link
     * HttpClient}クラスのインスタンスを指定すればよい。
     *
     * @param[in] token APIに送信するトークン
     * @param[in] origin APIサーバのオリジン
     * @param[in] baseUrl APIの起点となるURL
     * @param[in] client 通信を媒介するHttpClient
     */
    Fetch(std::string_view token,
          std::string_view origin,
          std::string_view baseUrl,
          HttpClientBase* client);
    virtual ~Fetch() noexcept;
    /**
     * {@inheritDoc}
     */
    virtual Result<_, ErrorResponse> init() override;
    /**
     * {@inheritDoc}
     */
    virtual FetchResult request(HttpMethod method, std::string_view url) override;
    /**
     * {@inheritDoc}
     */
    virtual FetchResult request(HttpMethod method,
                        std::string_view url,
                        const rapidjson::Document& body) override;
    /**
     * {@inheritDoc}
     */
    virtual FetchResult request(HttpMethod method,
                        std::string_view url,
                        std::string_view mimeType,
                        const std::vector<std::uint8_t>& body) override;

  private:
    /**
     * @brief Fetchクラスが内部的にHttpClientとの通信を行うためのメソッド。
     * @details
     * 各publicな{@link Fetch::request}は内部でこのメソッドをコールする。
     * 失敗した場合は次のエラーレスポンスを返す。
     * - ERR_JSON_PARSE_FAILED:
     * レスポンスのContent-Typeがapplication/jsonであったにもかかわらず正常なJSONデータがAPIから返却されなかったとき
     * - ERR_INCORRECT_HTTP_METHOD: POST, PUT以外のHTTPメソッドを指定したとき
     * - ERR_CURL_CONNECTION_FAILED: CURLの接続に失敗したとき
     *
     * @param[in] method リクエストに使用するHTTPメソッド
     * @param[in] origin APIのオリジン
     * @param[in] url APIへのURL。baseUrlを含む。
     * @param[in] headers リクエストのヘッダフィールド
     * @param[in] body APIリクエストのボディ部
     * @return FetchResult
     * 成功した場合はレスポンスのボディ部、失敗した場合は上記のエラーレスポンスを返す。
     *
     * @note
     * コンストラクタでオリジンを受け取っているにもかかわらず
     * ここでまたオリジンを受け取っていたり、
     * urlにbaseUrlを含ませる仕様にしているのはリダイレクト処理のため。
     */
    FetchResult request(HttpMethod method,
                        std::string_view origin,
                        std::string_view url,
                        const std::map<std::string, std::string>& headers,
                        const std::vector<std::uint8_t>& body);
  };
} // namespace octane::internal

#endif // OCTANE_API_CLIENT_INTERNAL_FETCH_H_