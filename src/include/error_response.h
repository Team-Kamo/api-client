#ifndef OCTANE_API_CLIENT_ERROR_RESPONSE_H_
#define OCTANE_API_CLIENT_ERROR_RESPONSE_H_
#include <string>
namespace octane {
  struct ErrorResponse {
    std::string code;
    std::string reason;
  };
} // namespace octane
#endif // OCTANE_API_CLIENT_ERROR_RESPONSE_H_