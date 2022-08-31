#include "include/api_client.h"

#include <gtest/gtest.h>

using namespace octane;

TEST(ApiClientTest, BasicAssertions) {
  ApiClient apiClient("mock","http://localhost:3000","/api/v1");
}
