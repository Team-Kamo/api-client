/**
 * @file error_response.h
 * @author soon (kento.soon@gmail.com)
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
  /**
   * @brief エラーを表す汎用構造体。
   *
   */
  struct ErrorResponse {
    /**
     * @brief エラーコード。
     * @details
     * {@link error_code.h}で列挙されている任意のエラーコードが格納される。
     * それ以外の値を入れることはなるべくやめてほしい。
     */
    std::string code;
    /**
     * @brief エラーが起こった原因。
     * @details
     * {@link
     * ErrorResponse::code}は機械向けのものだが、こちらは人間が読む用の文字列。
     * デバッグ効率を上げるためになるべく具体的な情報を入れたいところ。
     *
     */
    std::string reason;
  };

  /**
   * @brief エラーレスポンスを作成するためのユーティリティ関数。
   * @details
   * Result<T, ErrorResponse>という形式の戻り値をもつ関数で
   * より楽にエラーを返すために作られた。
   *
   * @code {.cpp}
   * // 通常次のように記述しなければならないところ
   * return error(ErrorResponse{
   *   .code = ERR_SOME_ERROR,
   *   .reason = "Error!!!",
   * });
   *
   * // 次のように記述できる
   * return makeError(ERR_SOME_ERROR, "Error!!!");
   * @endcode
   *
   * @param[in] code エラーコード。
   * @param[in] reason エラーの説明。
   * @return error_t<ErrorResponse> エラーを表すオブジェクト。
   */
  error_t<ErrorResponse> makeError(std::string_view code,
                                   std::string_view reason);
  std::ostream& operator<<(std::ostream& stream, const ErrorResponse& err);
} // namespace octane
#endif // OCTANE_API_CLIENT_ERROR_RESPONSE_H_