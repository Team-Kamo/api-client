#include "include/api_client.h"

#include <gtest/gtest.h>

using namespace octane;

TEST(ApiClientTest, BasicAssertions) {
  ApiClient apiClient;
  ASSERT_EQ(apiClient.test(20, 30), 50);
}
