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

#include <ostream>

namespace octane {
  error_t<ErrorResponse> makeError(std::string_view code,
                                   std::string_view reason) {
    return error(ErrorResponse{
      .code   = std::string(code),
      .reason = std::string(reason),
    });
  }
  bool operator==(const ErrorResponse& a, const ErrorResponse& b) {
    return (a.code == b.code && a.reason == b.reason);
  }
  std::ostream& operator<<(std::ostream& stream, const ErrorResponse& err) {
    stream << "{\n\t.code = " << err.code << "\n\t.reason = " << err.reason
           << "\n}";
    return stream;
  }
} // namespace octane