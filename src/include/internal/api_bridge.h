/**
 * @file api_bridge.h
 * @author soon (kento.soon@gmail.com)
 * @brief APIclientとAPI一つ一つを結びつける。
 * @version 0.1
 * @date 2022-09-01
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef OCTANE_API_CLIENT_INTERNAL_API_BRIDGE_H_
#define OCTANE_API_CLIENT_INTERNAL_API_BRIDGE_H_

#include "include/api_result_types.h"
#include "include/error_response.h"
#include "include/internal/fetch.h"
#include "include/result.h"

namespace octane::internal {
  class ApiBridge {
    FetchBase* fetch;

  public:
    /**
     * @brief Construct a new Api Bridge object
     * @details
     * {@link fetch} のコンストラクタに以下の引数を渡す。
     *
     * @param[in] fetch 通信に使用するfetchインスタンス
     */
    ApiBridge(FetchBase* fetch);
    /**
     * @brief Initialize
     * @details
     * このメソッドはインスタンス一つにつき一度だけ呼び出すことができる。
     * また、インスタンスを作成した直後に呼び出さなければならない。
     * 失敗した場合は次のエラーレスポンスを返す。
     * - ERR_CURL_INITIALIZATION_FAILED:
     * CURLの初期化に失敗したとき
     *
     * @return Result<std::optional<std::string>, ErrorResponse>
     * 成功した場合は何も返さず、失敗した場合は上記のエラーレスポンスを返す。
     */
    Result<_, ErrorResponse> init();
    /**
     * @brief use get method for /health
     *
     * @details
     * このメソッドは/healthにGETリクエストを発行する。
     * 失敗した場合は次のエラーレスポンスを返す。
     * - ERR_JSON_PARSE_FAILED:
     * レスポンスのContent-Typeがapplication/jsonであったにもかかわらず正常なJSONデータがAPIから返却されなかったとき
     * - ERR_INVALID_RESPONSE: レスポンスにエラーがあるとき
     * - ERR_CURL_CONNECTION_FAILED: CURLの接続に失敗したとき
     * また、2xx以外のレスポンスが返された時には、同様のエラーレスポンスの形式でサーバから渡ってきたエラーをそのまま返す。
     * @return Result<HealthResult, ErrorResponse>
     * 成功した場合はサーバの状態{@link
     * HealthResult}を返し、失敗した場合は上記のエラーレスポンスを返す。
     */
    Result<HealthResult, ErrorResponse> healthGet();
    /**
     * @brief use post method for /room
     * @details
     * このメソッドは/roomにPOSTリクエストを発行する。
     * 失敗した場合は次のエラーレスポンスを返す。
     * - ERR_JSON_PARSE_FAILED:
     * レスポンスのContent-Typeがapplication/jsonであったにもかかわらず正常なJSONデータがAPIから返却されなかったとき
     * - ERR_INVALID_RESPONSE: レスポンスにエラーがある時
     * - ERR_CURL_CONNECTION_FAILED: CURLの接続に失敗した時
     * また、2xx以外のレスポンスが返された時には、同様のエラーレスポンスの形式でサーバから渡ってきたエラーをそのまま返す。
     * @param[in] name ルームの名前
     * @return Result<RoomId, ErrorResponse>
     * 成功した場合はルームのid{@link
     * RoomId}を返し、失敗した場合は上記のエラーレスポンスを返す。
     */
    Result<RoomId, ErrorResponse> roomPost(std::string_view name);
    /**
     * @brief use get method for /room/{id}
     * @details
     * このメソッドは/room/{id}にGETリクエストを発行する。
     * 失敗した場合は次のエラーレスポンスを返す。
     * - ERR_JSON_PARSE_FAILED:
     * レスポンスのContent-Typeがapplication/jsonであったにもかかわらず正常なJSONデータがAPIから返却されなかったとき
     * - ERR_INVALID_RESPONSE: レスポンスにエラーがあるとき
     * - ERR_CURL_CONNECTION_FAILED: CURLの接続に失敗したとき
     * また、2xx以外のレスポンスが返された時には、同様のエラーレスポンスの形式でサーバから渡ってきたエラーをそのまま返す。
     * @param[in] id　ルームのid
     * @return Result<RoomStatus, ErrorResponse>
     * 成功した場合はルームのステータス{@link
     * RoomStatus}を返し、失敗した場合は上記のエラーレスポンスを返す。
     */
    Result<RoomStatus, ErrorResponse> roomIdGet(std::uint64_t id);
    /**
     * @brief use delete method for /room/{id}
     * @details
     * このメソッドは/room/{id}にDELETEリクエストを発行する。
     * 失敗した場合は次のエラーレスポンスを返す。
     * - ERR_CURL_CONNECTION_FAILED: CURLの接続に失敗したとき
     * また、2xx以外のレスポンスが返された時には、同様のエラーレスポンスの形式でサーバから渡ってきたエラーをそのまま返す。
     * @param[in] id ルームのid
     * @return Result<_, ErrorResponse>
     * 成功した場合には何も返さず、失敗した場合は上記のエラーレスポンスを返す。
     */
    Result<_, ErrorResponse> roomIdDelete(std::uint64_t id);
    /**
     * @brief use post method for /room/{id}
     * @details
     * このメソッドは/room/{id}にPOSTリクエストを発行する。
     * 失敗した場合は次のエラーレスポンスを返す。
     * - ERR_CURL_CONNECTION_FAILED: CURLの接続に失敗したとき
     * また、2xx以外のレスポンスが返された時には、同様のエラーレスポンスの形式でサーバから渡ってきたエラーをそのまま返す。
     * @param[in] id ルームのid
     * @param[in] name ルームに接続するデバイスの名前
     * @return Result<_, ErrorResponse>
     * 成功した場合には何も返さず、失敗した場合は上記のエラーレスポンスを返す。
     */
    Result<_, ErrorResponse> roomIdPost(std::uint64_t id,
                                        std::string_view name);
    /**
     * @brief use get method for /room/{id}/content
     * @details
     * このメソッドは/room/{id}/contentにGETリクエストを発行する。
     * 失敗した場合は次のエラーレスポンスを返す。
     * - ERR_INVALID_RESPONSE: レスポンスにエラーがあるとき
     * - ERR_CURL_CONNECTION_FAILED: CURLの接続に失敗したとき。
     * また、2xx以外のレスポンスが返された時には、同様のエラーレスポンスの形式でサーバから渡ってきたエラーをそのまま返す。
     * @param[in] id ルームのid
     * @return Result<std::vector<std::uint8_t>>,
     * ErrorResponse>
     * 成功した場合にはルーム内にあるバイナリデータを返し、失敗した場合には上記のエラーレスポンスを返す。
     */
    Result<std::vector<std::uint8_t>, ErrorResponse>
    roomIdContentGet(std::uint64_t id);
    /**
     * @brief use delete method for /room/{id}/content
     * @details
     * このメソッドは/room/{id}/contentにDELETEリクエストを発行する。
     * 失敗した場合は次のエラーレスポンスを返す。
     * - ERR_INVALID_RESPONSE: レスポンスにエラーがあるとき
     * - ERR_CURL_CONNECTION_FAILED: CURLの接続に失敗したとき。
     * また、2xx以外のレスポンスが返された時には、同様のエラーレスポンスの形式でサーバから渡ってきたエラーをそのまま返す。
     * @param[in] id ルームのid
     * @return Result<_, ErrorResponse>
     * 成功した場合には何も返さず、失敗した場合には上記のエラーレスポンスを返す。
     */
    Result<_, ErrorResponse> roomIdContentDelete(std::uint64_t id);
    /**
     * @brief use put method for /room/{id}/content
     * @details
     * このメソッドは/room/{id}/contentにPUTリクエストを発行する。
     * 失敗した場合は次のエラーレスポンスを返す。
     * - ERR_INVALID_RESPONSE: レスポンスにエラーがあるとき
     * - ERR_CURL_CONNECTION_FAILED: CURLの接続に失敗したとき。
     * また、2xx以外のレスポンスが返された時には、同様のエラーレスポンスの形式でサーバから渡ってきたエラーをそのまま返す。
     * @param[in] id ルームのid
     * @param[in] contentData ルームにアップロードするコンテンツのデータ
     * @param[in] mime ルームにアップロードするコンテンツのMIME
     * @return Result<_, ErrorResponse>
     * 成功した場合には何も返さず、失敗した場合には上記のエラーレスポンスを返す。
     */
    Result<_, ErrorResponse> roomIdContentPut(
      std::uint64_t id,
      const std::variant<std::string, std::vector<std::uint8_t>>& contentData,
      std::string_view mime);
    /**
     * @brief use get method for /room/{id}/status
     * @details
     * このメソッドは/room/{id}/statusにGETリクエストを発行する。
     * 失敗した場合は次のエラーレスポンスを返す。
     * - ERR_JSON_PARSE_FAILED:
     * レスポンスのContent-Typeがapplication/jsonであったにもかかわらず正常なJSONデータがAPIから返却されなかったとき
     * - ERR_INVALID_RESPONSE: レスポンスにエラーがあるとき
     * - ERR_CURL_CONNECTION_FAILED: CURLの接続に失敗したとき。
     * また、2xx以外のレスポンスが返された時には、同様のエラーレスポンスの形式でサーバから渡ってきたエラーをそのまま返す。
     * @param[in] id ルームのid
     * @return Result<ContentStatus, ErrorResponse>
     * 成功した場合にはコンテンツの状態{@link
     * ContentStatus}を返し、失敗した場合には上記のエラーレスポンスを返す。
     */
    Result<ContentStatus, ErrorResponse> roomIdStatusGet(std::uint64_t id);
    /**
     * @brief use delete method for /room/{id}/status
     * @details
     * このメソッドは/room/{id}/statusにDELETEリクエストを発行する。
     * 失敗した場合は次のエラーレスポンスを返す。
     * - ERR_INVALID_RESPONSE: レスポンスにエラーがあるとき
     * - ERR_CURL_CONNECTION_FAILED: CURLの接続に失敗したとき。
     * また、2xx以外のレスポンスが返された時には、同様のエラーレスポンスの形式でサーバから渡ってきたエラーをそのまま返す。
     * @param[in] id ルームのid
     * @return Result<_, ErrorResponse>
     * 成功した場合には何も返さず、失敗した場合には上記のエラーレスポンスを返す。
     */
    Result<_, ErrorResponse> roomIdStatusDelete(std::uint64_t id);
    /**
     * @brief use put method for /room/{id}/status
     * @details
     * このメソッドは/room/{id}/statusにDELETEリクエストを発行する。
     * 失敗した場合は次のエラーレスポンスを返す。
     * - ERR_INVALID_RESPONSE: レスポンスにエラーがあるとき
     * - ERR_CURL_CONNECTION_FAILED: CURLの接続に失敗したとき。
     * また、2xx以外のレスポンスが返された時には、同様のエラーレスポンスの形式でサーバから渡ってきたエラーをそのまま返す。
     * @param[in] id ルームのid
     * @param[in] contentStatus ルームにアップロードされているコンテンツの状態
     * @return Result<_, ErrorResponse>
     * 成功した場合には何も返さず、失敗した場合には上記のエラーレスポンスを返す。
     */
    Result<_, ErrorResponse> roomIdStatusPut(std::uint64_t id,
                                             const ContentStatus& contentStatus,
                                             std::string_view hash);
    /**
     * @brief check if the given status code is 2xx
     * @details
     * このメソッドはfetchしてきたレスポンスのステータスコードが2xxであるかどうかを確認する。
     * ステータスコードが2xxでない場合は、エラーレスポンスの形式でサーバから渡ってきたエラーをそのまま返す。
     * @param response fetchのレスポンス
     * @return std::optional<error_t<ErrorResponse>>
     * ステータスコードが2xxである場合には何も返さず、そうでない場合には上記のエラーレスポンスを返す。
     */
    std::optional<error_t<ErrorResponse>> checkStatusCode(
      const internal::FetchResponse& response);
  };
} // namespace octane::internal
#endif // OCTANE_API_CLIENT_INTERNAL_API_BRIDGE_H_