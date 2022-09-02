/**
 * @file api-client.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief APIクライアント。
 * @version 0.1
 * @date 2022-08-30
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef OCTANE_API_CLIENT_API_CLIENT_H_
#define OCTANE_API_CLIENT_API_CLIENT_H_

#include "./api_result_types.h"
#include "./config.h"
#include "./error_response.h"
#include "./internal/api_bridge.h"
#include "./internal/hash.h"
#include "./result.h"
namespace octane {
  class ApiClient {
    internal::ApiBridge bridge;
    std::uint64_t lastCheckedTime;

  public:
    /**
     * @brief Construct a new Api Client object
     *
     * @param[in] token
     * @param[in] origin http://localhost:3000
     * @param[in] baseUrl /api/v1
     */
    ApiClient(std::string_view token   = DEFAULT_API_TOKEN,
              std::string_view origin  = DEFAULT_API_ORIGIN,
              std::string_view baseUrl = DEFAULT_API_BASE_URL);
    /**
     * @brief Destroy the Api Client object
     *
     */
    ~ApiClient() noexcept;

    /**
     * @brief Initialize
     *
     * @return Result<std::optional<std::string>, ErrorResponse>
     */
    Result<std::optional<std::string>, ErrorResponse> init();

    /**
     * @brief Return the server's status
     *
     * @return Result<HealthResult, ErrorResponse>
     */
    Result<HealthResult, ErrorResponse> health();
    /**
     * @brief Create a room
     *
     * @param[in] name Room name
     * @return Result<std::string, ErrorResponse> Returns room id on success
     */
    Result<std::string, ErrorResponse> createRoom(std::string_view name);
    /**
     * @brief Get the room's status
     *
     * @param[in] id
     * @return Result<RoomStatus, ErrorResponse>
     */
    Result<RoomStatus, ErrorResponse> getRoomStatus(std::string_view id);
    /**
     * @brief Delete the room
     *
     * @param[in] id
     * @return Result<_, ErrorResponse>
     */
    Result<_, ErrorResponse> deleteRoom(std::string_view id);
    /**
     * @brief Return content in the room
     * 
     * @param[in] id 
     * @param[in] name 
     * @return Result<Content, ErrorResponse> 
     */
    Result<Content, ErrorResponse> getContent(std::string_view id,
                                              std::string_view name);
    /**
     * @brief Delete content from the room
     * 
     * @param id 
     * @param name 
     * @return Result<_, ErrorResponse> 
     */
    Result<_, ErrorResponse> deleteContent(std::string_view id,
                                           std::string_view name);
    /**
     * @brief Upload content to the room
     *
     * @param[in] id
     * @param[in] name
     * @return Result<_, ErrorResponse>
     */
    Result<_, ErrorResponse> uploadContent(std::string_view id,
                                           std::string_view name,
                                           const Content& content);

  private:
    Result<std::optional<std::string>, ErrorResponse> checkHealth();
    /**
     * @brief Connect to the room
     * @details The user doesn't have to call this method since it's only used
     * in api_client
     * @param[in] id
     * @param name
     * @return Result<_,ErrorResponse>
     */
  };
} // namespace octane

#endif // OCTANE_API_CLIENT_API_CLIENT_H_