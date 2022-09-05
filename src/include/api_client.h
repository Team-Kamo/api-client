/**
 * @file api-client.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief  Provides simple methods to communicate with OctaneServer.
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
    HealthResult lastCheckedHealth;
    struct ConnectionStatus {
      bool isConnected;
      /**
       * @brief Room id which the user is connected to.
       *
       */
      std::uint64_t id;
      /**
       * @brief An unique name for the device which is connected to the room.
       *
       */
      std::string name;
    };
    ConnectionStatus connectionStatus{ .isConnected = false };

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
     * @brief Run this method at first.(Since no exeptions are allowed in the
     * constructor, we need this method to initailize the system.)
     * @details
     * This method can be called once for each instance, and should be called
     * right after an instance is created.
     * If it fails, the following error response will be returned.
     * - ERR_CURL_INITIALIZATION_FAILED
     * - ERR_JSON_PARSE_FAILED
     * - ERR_INVALID_RESPONSE
     * - ERR_CURL_CONNECTION_FAILED
     * - ERR_SERVER_HEALTH_STATUS_FAULTY
     * Additionaly, when a response other than a 2xx is returned, the error
     * passed from the server in the form of error response is returned.
     * @return Result<std::optional<std::string>, ErrorResponse>
     * On success, it will return {@link Response}.
     * On failure, it will return the error response written above.
     */
    Result<Response, ErrorResponse> init();
    /**
     * @brief Creates a room
     * @details
     * This method creates a room, and
     * returns {@link RoomId} when you pass the room name. If it fails, the
     * following error response will be returned.
     * - ERR_JSON_PARSE_FAILED
     * - ERR_INVALID_RESPONSE
     * - ERR_CURL_CONNECTION_FAILED
     * - ERR_SERVER_HEALTH_STATUS_FAULTY
     * Additionaly, when a response other than 2xx is returned, the
     * error passed from the server in the form of error response is returned.
     * @param[in] name Room name
     * @return Result<RoomId, ErrorResponse>
     * On success, it will return {@link RoomId}.
     * On failure, it will return the error response written above.
     */
    Result<RoomId, ErrorResponse> createRoom(std::string_view name);
    /**
     * @brief Connects to the room
     * @details
     * This method connects to the room when you pass the room id and device
     * name. This method should not be called after you are connected. If it
     * fails, the following error response will be returned.
     * - ERR_CURL_CONNECTION_FAILED
     * - ERR_SERVER_HEALTH_STATUS_FAULTY
     * Additionaly, when a response other than 2xx is returned, the
     * error passed from the server in the form of error response is returned.
     * @param[in] id Room id
     * @param[in] name Device name
     * @return Result<Response, ErrorResponse>
     * On success, it will return {@link Response}.
     * On failure, it will return the error respose written above.
     */
    Result<Response, ErrorResponse> connectRoom(std::uint64_t id,
                                                std::string_view name);
    /**
     * @brief Disconnects from the room
     * @details
     * This method disconnects from the room when you pass the room id and
     * device name. This method should not be called after you are disconnected.
     * If it fails, the following error response will be returned.
     * - ERR_CURL_CONNECTION_FAILED
     * - ERR_SERVER_HEALTH_STATUS_FAULTY
     * Additionaly, when a response other than 2xx is returned, the
     * error passed from the server in the form of error response is returned.
     * @param[in] id Room id
     * @param[in] name Device name
     * @return Result<Response, ErrorResponse>
     * On success, it will return {@link Response}.
     * On failure, it will return the error respose written above.
     */
    Result<Response, ErrorResponse> disconnectRoom(std::uint64_t id,
                                                   std::string_view name);
    /**
     * @brief Gets the room's status
     * @details
     * This method returns {@link RoomStatus} when you pass the room id.
     * If you pass nothing and you are connected to a room, it will return the
     * {@link RoomStatus} for that room. If it fails, the following error
     * response will be returned.
     * - ERR_JSON_PARSE_FAILED
     * - ERR_INVALID_RESPONSE
     * - ERR_CURL_CONNECTION_FAILED
     * - ERR_SERVER_HEALTH_STATUS_FAULTY
     * - ERR_ROOM_ID_UNDEFINED
     * Additionaly, when a response other than 2xx is returned, the
     * error passed from the server in the form of error response is returned.
     * @param[in] id Room id
     * @return Result<RoomStatus, ErrorResponse>
     * On success, it will return {@link RoomStatus}.
     * On failure, it will return the error response written above.
     */
    Result<RoomStatus, ErrorResponse> getRoomStatus(
      std::optional<std::uint64_t> id = std::nullopt);
    /**
     * @brief Deletes the room
     * @details
     * This method deletes the room when you pass the room id.
     * If you pass nothing and you are connected to a room, it will delete that
     * room. If it fails, the following error response will be returned.
     * - ERR_CURL_CONNECTION_FAILED
     * - ERR_SERVER_HEALTH_STATUS_FAULTY
     * Additionaly, when a response other than 2xx is returned, the
     * error passed from the server in the form of error response is returned.
     * @param[in] id Room id
     * @return Result<Response, ErrorResponse>
     * On success, it will return {@link Response}
     * On failure, it will return the error response written above.
     */
    Result<Response, ErrorResponse> deleteRoom(std::optional<std::uint64_t> id
                                               = std::nullopt);
    /**
     * @brief Gets the room's content
     * @details
     * This method returns the room's {@link Content}.
     * If it fails, the following error response will be returned.
     * - ERR_JSON_PARSE_FAILED
     * - ERR_INVALID_RESPONSE
     * - ERR_CURL_CONNECTION_FAILED
     * - ERR_SERVER_HEALTH_STATUS_FAULTY
     * - ERR_ROOM_DISCONNECTED
     * Additionaly, when a response other than 2xx is returned, the
     * error passed from the server in the form of error response is returned.
     * @return Result<Content, ErrorResponse>
     * On success, it will return {@link Content}.
     * On failure, it will return the error response written above.
     */
    Result<Content, ErrorResponse> getContent();
    /**
     * @brief Deletes the room's content
     * @details
     * This method deletes the room's {@link Content}.
     *  If it fails, the following error response will be returned.
     * - ERR_CURL_CONNECTION_FAILED
     * - ERR_SERVER_HEALTH_STATUS_FAULTY
     * - ERR_ROOM_DISCONNECTED
     * Additionaly, when a response other than 2xx is returned, the
     * error passed from the server in the form of error response is returned.
     * @return Result<Response, ErrorResponse>
     * On success, it will return {@link Response}.
     * On failure, it will return the error response written above.
     */
    Result<Response, ErrorResponse> deleteContent();
    /**
     * @brief Uploads content to the room
     * @details
     * This method uploads the content (file or clipboard) to the room, by
     * passing it {@link Content}. If it fails, the following error response
     * will be returned.
     * - ERR_CURL_CONNECTION_FAILED
     * - ERR_SERVER_HEALTH_STATUS_FAULTY
     * - ERR_ROOM_DISCONNECTED
     * Additionaly, when a response other than 2xx is returned, the
     * error passed from the server in the form of error response is returned.
     * @param[in] content Content you want to upload
     * @return Result<Response, ErrorResponse>
     * On success, it will return {@link Response}.
     * On failure, it will return the error response written above.
     */
    Result<Response, ErrorResponse> uploadContent(const Content& content);

  private:
    /**
     * @brief Returns the server's status
     * @details
     * This pricate method returns the server's status in type {@link
     * HealthResult}. If it fails, the following error response will be
     * returned.
     * - ERR_JSON_PARSE_FAILED
     * - ERR_INVALID_RESPONSE
     * - ERR_CURL_CONNECTION_FAILED
     * - ERR_SERVER_HEALTH_STATUS_FAULTY
     * Additionaly, when a response other than 2xx is returned, the
     * error passed from the server in the form of error response is returned.
     * @return Result<HealthResult, ErrorResponse>
     * On success, it will return {@link HealthResult}.
     * On failure, it will return the error response written above.
     */
    Result<HealthResult, ErrorResponse> health();
    /**
     * @brief Calls health if this method was previously called more than 30
     * minutes ago
     * @details
     * This private method checks if the client has called this method more than
     * 30 minutes ago, and if so, this method calls health, and if the server is
     * not faulty, it returns {@link HealthResult}.
     * If it fails, the following error response will be returned.
     * - ERR_JSON_PARSE_FAILED
     * - ERR_INVALID_RESPONSE
     * - ERR_CURL_CONNECTION_FAILED
     * - ERR_SERVER_HEALTH_STATUS_FAULTY
     * Additionaly, when a response other than 2xx is returned, the
     * error passed from the server in the form of error response is returned.
     * @return Result<HealthResult, ErrorResponse>
     * On success, it will return {@link HealthResult}.
     * On failure, it will return the error response written above.
     */
    Result<HealthResult, ErrorResponse> checkHealth();
    /**
     * @brief Checks if the input is binary, and if not, returns the
     * binary-nized input.
     *
     * @param[in] input
     * @return std::vector<std::uint8_t>
     */
    std::vector<std::uint8_t> createBinary(
      const std::variant<std::string,
                         std::vector<uint8_t>,
                         std::vector<FileInfo>>& input);
  };
} // namespace octane

#endif // OCTANE_API_CLIENT_API_CLIENT_H_