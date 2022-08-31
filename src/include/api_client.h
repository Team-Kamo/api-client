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

#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "./error_response.h"
#include "./fetch.h"
#include "./result.h"

namespace octane {
  enum struct Health {
    Healthy,
    Degraded,
    Faulty,
  };
  struct HealthResult {
    Health health;
    std::optional<std::string> message;
  };
  struct Device {
    std::string name;
    std::uint64_t timestamp;
  };
  struct RoomStatus {
    std::string name;
    std::vector<Device> devices;
  };
  enum struct ContentType {
    File,
    Clipboard,
  };
  struct Content {
    ContentType type;
    std::string name;
    std::string mime;
    std::variant<std::string, std::vector<std::uint8_t>> data;
  };
  struct ContentStatus {
    std::string device;
    std::uint64_t timestamp;
    ContentType type;
    std::optional<std::string> name;
    std::string mime;
    std::string hash;
  };
  class ApiClient {
    internal::Fetch fetch;
    std::uint64_t lastCheckedTime;

  public:
    /**
     * @brief Construct a new Api Client object
     *
     * @param token
     * @param origin http://localhost:3000
     * @param baseUrl /api/v1
     */
    ApiClient(std::string_view token, std::string_view origin, std::string_view baseUrl);
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
     * @param name Room name
     * @return Result<std::string, ErrorResponse> Returns room id on success
     */
    Result<std::string, ErrorResponse> createRoom(std::string_view name);
    /**
     * @brief Get the room's status
     *
     * @param id
     * @return Result<RoomStatus, ErrorResponse>
     */
    Result<RoomStatus, ErrorResponse> getRoomStatus(std::string_view id);
    /**
     * @brief Delete the room
     *
     * @param id
     * @return Result<_, ErrorResponse>
     */
    Result<_, ErrorResponse> deleteRoom(std::string_view id);
    /**
     * @brief Return content in the room
     *
     * @return Result<Content, ErrorResponse>
     */
    Result<Content, ErrorResponse> getContent(std::string_view id);
    /**
     * @brief Delete content from the room
     *
     * @param id
     * @return Result<_, ErrorResponse>
     */
    Result<_, ErrorResponse> deleteContent(std::string_view id);
    /**
     * @brief Upload content to the room
     *
     * @return Result<_, ErrorResponse>
     */
    Result<_, ErrorResponse> uploadContent(std::string_view id,
                                           const Content& content);

  private:
    Result<std::optional<std::string>, ErrorResponse> checkHealth();
    /**
     * @brief Connect to the room
     * @details The user doesn't have to call this method since it's only used
     * in api_client
     * @param id
     * @param name
     * @return Result<_,ErrorResponse>
     */
    Result<_, ErrorResponse> connectRoom(std::string_view id,
                                         std::string_view name);
    /**
     * @brief Return content status in the room
     *
     * @param id
     * @return Result<ContentStatus, ErrorResponse>
     */
    Result<ContentStatus, ErrorResponse> getContentStatus(std::string_view id);
    /**
     * @brief Upload content status to the room
     *
     * @param id
     * @return Result<_, ErrorResponse>
     */
    Result<_, ErrorResponse> uploadContentStatus(
      std::string_view id,
      const ContentStatus& contentStatus);
    /**
     * @brief Delete content status from the room
     *
     * @param id
     * @return Result<_, ErrorResponse>
     */
    Result<_, ErrorResponse> deleteContentStatus(std::string_view id);
  };
} // namespace octane

#endif // OCTANE_API_CLIENT_API_CLIENT_H_