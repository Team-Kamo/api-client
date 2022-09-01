/**
 * @file error_response.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief error_response.hの実装。
 * @version 0.1
 * @date 2022-08-31
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "include/error_response.h"

namespace octane {
  error_t<ErrorResponse> makeError(std::string_view code,
                                   std::string_view reason) {
    return error(ErrorResponse{
      .code   = std::string(code),
      .reason = std::string(reason),
    });
  }
} // namespace octane