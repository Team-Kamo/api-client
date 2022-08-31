#ifndef OCTANE_API_CLIENT_FETCH_H_
#define OCTANE_API_CLIENT_FETCH_H_

#include <rapidjson/document.h>
#include <rapidjson/encodings.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <map>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "./http_client.h"
#include "include/error_response.h"
#include "include/result.h"

namespace octane::internal {
  class Fetch {
    std::string token;
    std::string origin;
    std::string baseUrl;

    HttpClientBase* client;

  public:
    using FetchResult
      = Result<std::variant<rapidjson::Document, std::vector<std::uint8_t>>,
               ErrorResponse>;

    /**
     * @brief Construct a new Fetch object
     *
     * @param token
     * @param origin http://localhost:3000
     * @param baseUrl /api/v1
     * @param client
     */
    Fetch(std::string_view token,
          std::string_view origin,
          std::string_view baseUrl,
          HttpClientBase* client);
    /**
     * @brief Initailize
     *
     * @return Result<_, ErrorResponse>
     */
    Result<_, ErrorResponse> init();
    /**
     * @brief Requests
     *
     * @param method
     * @param url
     * @return FetchResult
     */
    FetchResult request(HttpMethod method, std::string_view url);
    /**
     * @brief Requests
     *
     * @param method
     * @param url
     * @param body
     * @return FetchResult
     */
    FetchResult request(HttpMethod method,
                        std::string_view url,
                        const rapidjson::Document& body);
    /**
     * @brief Requests
     *
     * @param method
     * @param url
     * @param body
     * @return FetchResult
     */
    FetchResult request(HttpMethod method,
                        std::string_view url,
                        std::string_view mimeType,
                        const std::vector<std::uint8_t>& body);

  private:
    /**
     * @brief Requests
     *
     * @param method
     * @param url
     * @param headers
     * @param body
     * @return FetchResult
     */
    FetchResult request(HttpMethod method,
                        std::string_view url,
                        const std::map<std::string, std::string>& headers,
                        const std::vector<std::uint8_t>& body);
  };
} // namespace octane::internal

#endif // OCTANE_API_CLIENT_FETCH_H_