#ifndef OCTANE_API_CLIENT_INTERNAL_MULTI_FILE_H_
#define OCTANE_API_CLIENT_INTERNAL_MULTI_FILE_H_

#include <vector>

#include "../api_result_types.h"
#include "../error_response.h"
#include "../result.h"

namespace octane::internal {
  class MultiFileCompressor {
  public:
    static Result<std::vector<uint8_t>, ErrorResponse> compress(
      const std::vector<FileInfo>& files);
  };
  class MultiFileDecompressor {
  public:
    static Result<std::vector<FileInfo>, ErrorResponse> decompress(
      const std::vector<uint8_t>& data);
  };
} // namespace octane::internal

#endif // OCTANE_API_CLIENT_INTERNAL_MULTI_FILE_H_