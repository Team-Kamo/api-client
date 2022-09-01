#ifndef OCTANE_API_CLIENT_ERROR_CODE_H_
#define OCTANE_API_CLIENT_ERROR_CODE_H_

namespace octane {
  constexpr auto ERR_API_CLIENT_INITIALIZATION_FAILED = "ERR_API_CLIENT_INITIALIZATION_FAILED";
  constexpr auto ERR_SERVER_HEALTH_STATUS_FAULTY = "ERR_SERVER_HEALTH_STATUS_FAULTY";
  constexpr auto ERR_CURL_INITIALIZATION_FAILED = "ERR_CURL_INITIALIZATION_FAILED";
  constexpr auto ERR_INCORRECT_HTTP_METHOD = "ERR_INCORRECT_HTTP_METHOD";
  constexpr auto ERR_CURL_CONNECTION_FAILED = "ERR_CURL_CONNECTION_FAILED";
  constexpr auto ERR_JSON_PARSE_FAILED = "ERR_JSON_PARSE_FAILED";
  constexpr auto ERR_INVALID_RESPONSE = "ERR_INVALID_RESPONSE";
  constexpr auto ERR_INVALID_REQUEST = "ERR_INVALID_REQUEST";
}

#endif // OCTANE_API_CLIENT_ERROR_CODE_H_