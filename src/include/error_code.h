/**
 * @file error_code.h
 * @author soon (kento.soon@gmail.com)
 * @brief Define error codes.
 * @version 0.1
 * @date 2022-09-01
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef OCTANE_API_CLIENT_ERROR_CODE_H_
#define OCTANE_API_CLIENT_ERROR_CODE_H_

namespace octane {
  /** @brief Used when {@link ApiClient} initialization failed.*/
  constexpr auto ERR_API_CLIENT_INITIALIZATION_FAILED
    = "ERR_API_CLIENT_INITIALIZATION_FAILED";
  /** @brief Used when the server has incidents and is currently not available.
   */
  constexpr auto ERR_SERVER_HEALTH_STATUS_FAULTY
    = "ERR_SERVER_HEALTH_STATUS_FAULTY";
  /** @brief Used when cURL initialization failed, such as in{@link
   * octane::internal::HttpClient}。
   */
  constexpr auto ERR_CURL_INITIALIZATION_FAILED
    = "ERR_CURL_INITIALIZATION_FAILED";
  /** @brief Used when an unexpected http method was issued. */
  constexpr auto ERR_INCORRECT_HTTP_METHOD = "ERR_INCORRECT_HTTP_METHOD";
  /** @brief Used when cURL failed to connect. */
  constexpr auto ERR_CURL_CONNECTION_FAILED = "ERR_CURL_CONNECTION_FAILED";
  /** @brief Used when JSON parse failed. */
  constexpr auto ERR_JSON_PARSE_FAILED = "ERR_JSON_PARSE_FAILED";
  /** @brief Used when there was an unexpected response from the server. */
  constexpr auto ERR_INVALID_RESPONSE = "ERR_INVALID_RESPONSE";
  /** @brief Used when an unexpected request was issued. */
  constexpr auto ERR_INVALID_REQUEST = "ERR_INVALID_REQUEST";
  /** @brief Used when you are not connected to a room but {@link
   * getRoomStatus()} and {@link deleteRoom()} was passed nothing. */
  constexpr auto ERR_ROOM_ID_UNDEFINED     = "ERR_ROOM_ID_UNDEFINED";
  constexpr auto ERR_ROOM_DISCONNECTED     = "ERR_ROOM_DISCONNECTED";
  constexpr auto ERR_DUP_DEVICE            = "ERR_DUP_DEVICE";
  constexpr auto ERR_CONTENT_HASH_MISMATCH = "ERR_CONTENT_HASH_MISMATCH";
} // namespace octane

#endif // OCTANE_API_CLIENT_ERROR_CODE_H_