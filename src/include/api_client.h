/**
 * @file api-client.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-30
 *
 * APIクライアント。
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef OCTANE_API_CLIENT_API_CLIENT_H_
#define OCTANE_API_CLIENT_API_CLIENT_H_

namespace octane {
  class ApiClient {
  public:
    ApiClient();
    ~ApiClient() noexcept;
    int test(int a, int b);
  };
} // namespace octane

#endif // OCTANE_API_CLIENT_API_CLIENT_H_