#ifndef OCTANE_API_CLIENT_TEST_MOCK_MOCK_FETCH_H_
#define OCTANE_API_CLIENT_TEST_MOCK_MOCK_FETCH_H_

#include <gmock/gmock.h>

#include "include/internal/fetch.h"

namespace octane::test {
  class MockFetch : public internal::FetchBase {
  public:
    MOCK_METHOD((Result<_, ErrorResponse>), init, (), ());
    MOCK_METHOD((internal::Fetch::FetchResult),
                request,
                (internal::HttpMethod method, std::string_view url),
                (override));
    MOCK_METHOD((internal::Fetch::FetchResult),
                request,
                (internal::HttpMethod method, std::string_view url, const rapidjson::Document& body),
                (override));
    MOCK_METHOD((internal::Fetch::FetchResult),
                request,
                (internal::HttpMethod method, std::string_view url, std::string_view mimeType, const std::vector<std::uint8_t>& body),
                (override));
  };
} // namespace octane::test

#endif // OCTANE_API_CLIENT_TEST_MOCK_MOCK_FETCH_H_