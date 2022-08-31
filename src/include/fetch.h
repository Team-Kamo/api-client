#ifndef OCTANE_API_CLIENT_FETCH_H_
#define OCTANE_API_CLIENT_FETCH_H_

#include <rapidjson/document.h>

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

  public:
    /**
     * @brief Construct a new Fetch object
     * 
     * @param token 
     * @param origin 
     */
    Fetch(std::string_view token, std::string_view origin);
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
     * @brief Requests ,used for GET and DELETE
     * 
     * @param method 
     * @param url 
     * @return Result<std::variant<rapidjson::Document, std::vector<std::uint8_t>>,
     * ErrorResponse> 
     */
    Result<std::variant<rapidjson::Document, std::vector<std::uint8_t>>,
           ErrorResponse>
    request(HttpMethod method, std::string_view url);
    /**
     * @brief used for POST and PUT methods, 
     * 
     * @param method 
     * @param url 
     * @param body 
     * @return Result<std::variant<rapidjson::Document, std::vector<std::uint8_t>>,
     * ErrorResponse> 
     */
    Result<std::variant<rapidjson::Document, std::vector<std::uint8_t>>,
           ErrorResponse>
    request(HttpMethod method,
            std::string_view url,
            const rapidjson::Document& body);
    /**
     * @brief 
     * 
     * @param method 
     * @param url 
     * @param body 
     * @return Result<std::variant<rapidjson::Document, std::vector<std::uint8_t>>,
     * ErrorResponse> 
     */
    Result<std::variant<rapidjson::Document, std::vector<std::uint8_t>>,
           ErrorResponse>
    request(HttpMethod method,
            std::string_view url,
            const std::vector<std::uint8_t>& body);
  };
} // namespace octane::internal

#endif // OCTANE_API_CLIENT_FETCH_H_