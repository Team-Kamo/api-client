/**
 * @file error_response.h
 * @author soon (kento.soon@gmail.com)
 * @brief Error response.
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
   * @brief General stucture to represent the error.
   *
   */
  struct ErrorResponse {
    /**
     * @brief Error code.
     * @details
     * {@link error_code.h}で列挙されている任意のエラーコードが格納される。
     * それ以外の値を入れることはなるべくやめてほしい。
     * Error code listed in {@link error_code.h} will be stored.
     * Don't try to substitute other values.
     */
    std::string code;
    /**
     * @brief Reason why the error ocurred.
     * @details
     * {@link
     * ErrorResponse::code}は機械向けのものだが、こちらは人間が読む用の文字列。
     * デバッグ効率を上げるためになるべく具体的な情報を入れたいところ。
     * {@link ErrorResponse::code} is for machines, but this is strings for
     * human beings to read. We want to include as much specific information as
     * possible to improve debugging efficiency.
     *
     */
    std::string reason;
  };
  /**
   * @brief Utility function to create error response.
   * @details
   * Result<T, ErrorResponse>という形式の戻り値をもつ関数で
   * より楽にエラーを返すために作られた。
   * Created to make it easier to return errors in functions which return
   * Result<T, ErrorResponse>.
   *
   * @code {.cpp}
   * // 通常次のように記述しなければならないところ
   * // Usually, we had to write error response like this,
   * return error(ErrorResponse{
   *   .code = ERR_SOME_ERROR,
   *   .reason = "Error!!!",
   * });
   * // 次のように記述できる
   * // But now it can be written as following.
   * return makeError(ERR_SOME_ERROR, "Error!!!");
   * @endcode
   *
   * @param[in] code Error code.
   * @param[in] reason Reason why the error ocurred.
   * @return error_t<ErrorResponse> Object represesnting error.
   */
  error_t<ErrorResponse> makeError(std::string_view code,
                                   std::string_view reason);
  std::ostream& operator<<(std::ostream& stream, const ErrorResponse& err);
} // namespace octane
#endif // OCTANE_API_CLIENT_ERROR_RESPONSE_H_