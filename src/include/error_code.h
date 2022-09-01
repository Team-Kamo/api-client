/**
 * @file error_code.h
 * @author soon (kento.soon@gmail.com)
 * @brief エラーコードを定義する。
 * @version 0.1
 * @date 2022-09-01
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef OCTANE_API_CLIENT_ERROR_CODE_H_
#define OCTANE_API_CLIENT_ERROR_CODE_H_

namespace octane {
  /** @brief {@link ApiClient}の初期化に失敗したときに使用される。*/
  constexpr auto ERR_API_CLIENT_INITIALIZATION_FAILED = "ERR_API_CLIENT_INITIALIZATION_FAILED";
  /** @brief サーバに障害が発生し、正常に動作できないときに使用される。 */
  constexpr auto ERR_SERVER_HEALTH_STATUS_FAULTY = "ERR_SERVER_HEALTH_STATUS_FAULTY";
  /** @brief {@link octane::internal::HttpClient}などでCURLの初期化に失敗したときに使用される。 */
  constexpr auto ERR_CURL_INITIALIZATION_FAILED = "ERR_CURL_INITIALIZATION_FAILED";
  /** @brief 予期しないHTTPリクエストを発行したときに使用される。 */
  constexpr auto ERR_INCORRECT_HTTP_METHOD = "ERR_INCORRECT_HTTP_METHOD";
  /** @brief CURLの処理に失敗したときに使用される。 */
  constexpr auto ERR_CURL_CONNECTION_FAILED = "ERR_CURL_CONNECTION_FAILED";
  /** @brief JSONのパースに失敗したときに使用される。 */
  constexpr auto ERR_JSON_PARSE_FAILED = "ERR_JSON_PARSE_FAILED";
  /** @brief APIサーバから予期しない応答があったときに使用される。 */
  constexpr auto ERR_INVALID_RESPONSE = "ERR_INVALID_RESPONSE";
  /** @brief 予期しないリクエストを発行したときに使用される。 */
  constexpr auto ERR_INVALID_REQUEST = "ERR_INVALID_REQUEST";
  /** @brief 予期しないステータスコードが返されたときに使用される。 */
  constexpr auto ERR_INVALID_STATUS_CODE = "ERR_INVALID_STATUS_CODE";
}

#endif // OCTANE_API_CLIENT_ERROR_CODE_H_