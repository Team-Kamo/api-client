#ifndef OCTANE_API_CLIENT_TEST_MOCK_MOCK_HTTP_CLIENT_H_
#define OCTANE_API_CLIENT_TEST_MOCK_MOCK_HTTP_CLIENT_H_

#include <gmock/gmock.h>

#include "include/internal/http_client.h"

namespace octane::test {
  class MockHttpClient : public octane::internal::HttpClientBase {
  public:
    MOCK_METHOD((octane::Result<_, octane::ErrorResponse>),
                init,
                (),
                (noexcept));

    MOCK_METHOD(
      (octane::Result<octane::internal::HttpResponse, octane::ErrorResponse>),
      request,
      (std::string_view, const octane::internal::HttpRequest&));
  };
} // namespace octane::test

#endif // OCTANE_API_CLIENT_TEST_MOCK_MOCK_HTTP_CLIENT_H_