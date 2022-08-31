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

#include "./error_response.h"
#include "./result.h"

namespace octane::internal {
  enum struct HttpMethod {
    Get,
    Post,
    Put,
    Delete,
  };
  class Fetch {
    std::string token;
    std::string origin;
    std::string baseUrl;
  public:
    /**
     * @brief Construct a new Fetch object
     *
     * @param token
     * @param origin http://localhost:3000
     * @param baseUrl /api/v1
     */
    Fetch(std::string_view token, std::string_view origin, std::string_view baseUrl);
    /**
     * @brief Destroy the Fetch object
     *
     */
    ~Fetch();
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
     * @return Result<std::variant<rapidjson::Document,
     * std::vector<std::uint8_t>>, ErrorResponse>
     */
    Result<std::variant<rapidjson::Document, std::vector<std::uint8_t>>,
           ErrorResponse>
    request(HttpMethod method, std::string_view url);
    /**
     * @brief Requests
     *
     * @param method
     * @param url
     * @param body
     * @return Result<std::variant<rapidjson::Document,
     * std::vector<std::uint8_t>>, ErrorResponse>
     */
    Result<std::variant<rapidjson::Document, std::vector<std::uint8_t>>,
           ErrorResponse>
    request(HttpMethod method,
            std::string_view url,
            const rapidjson::Document& body);
    /**
     * @brief Requests
     *
     * @param method
     * @param url
     * @param body
     * @return Result<std::variant<rapidjson::Document,
     * std::vector<std::uint8_t>>, ErrorResponse>
     */
    Result<std::variant<rapidjson::Document, std::vector<std::uint8_t>>,
           ErrorResponse>
    request(HttpMethod method,
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
     * @return Result<std::variant<rapidjson::Document,
     * std::vector<std::uint8_t>>, ErrorResponse>
     */
    Result<std::variant<rapidjson::Document, std::vector<std::uint8_t>>,
           ErrorResponse>
    request(HttpMethod method,
            std::string_view url,
            const std::map<std::string, std::string>& headers,
            const std::vector<std::uint8_t>& body);
    /**
     * @brief callback used for curl
     *
     * @param buffer
     * @param size
     * @param nmemb
     * @param chunk
     * @return size_t
     */
    static size_t writeCallback(char* buffer,
                                size_t size,
                                size_t nmemb,
                                std::vector<std::uint8_t>* chunk);
    static size_t readCallback(
      char* buffer,
      size_t size,
      size_t nmemb,
      std::pair<const std::vector<uint8_t>*, size_t>* stream);
    static size_t headerCallback(char* buffer,
                                 size_t size,
                                 size_t nmemb,
                                 std::map<std::string, std::string>* returnHeaders);
  };
} // namespace octane::internal

#endif // OCTANE_API_CLIENT_FETCH_H_