/**
 * @file error_response.h
 * @author res6idue (kento.soon@gmail.com)
 * @brief エラーレスポンス。
 * @version 0.1
 * @date 2022-08-31
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef OCTANE_API_CLIENT_ERROR_RESPONSE_H_
#define OCTANE_API_CLIENT_ERROR_RESPONSE_H_

#include <string>
#include <string_view>

#include "./result.h"

namespace octane {
  struct ErrorResponse {
    std::string code;
    std::string reason;
  };

  /**
   * @brief エラーレスポンスを作成するためのユーティリティ関数。
   * @details
   * Result<T, ErrorResponse>という形式の戻り値をもつ関数で
   * より楽にエラーを返すために作られた。
   *
   * @param[in] code エラーコード。
   * @param[in] reason エラーの説明。
   * @return error_t<ErrorResponse> エラーを表すオブジェクト。
   */
  error_t<ErrorResponse> makeError(std::string_view code,
                                   std::string_view reason);
} // namespace octane
#endif // OCTANE_API_CLIENT_ERROR_RESPONSE_H_