/**
 * @file api_result_types.h
 * @author soon (kento.soon@gmail.com)
 * @brief Types which are used to return  in api client.
 * @version 0.1
 * @date 2022-09-01
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef OCTANE_API_CLIENT_API_RESULT_TYPES_H_
#define OCTANE_API_CLIENT_API_RESULT_TYPES_H_

#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "./result.h"

namespace octane {
  /**
   * @brief Enum used in {@link HealthResult}, represents the server's status.
   *
   */
  enum struct Health {
    /** @brief The server is working. */
    Healthy,
    /** @brief There are incidents happening in the server. */
    Degraded,
    /** @brief The server is dead. */
    Faulty,
  };
  /**
   * @brief Structure used as result for {@link health}, has the server's status
   * and message.
   *
   */
  struct HealthResult {
    /** @brief Status of the server. */
    Health health;
    /** @brief Message which describe details of the server's status. */
    std::optional<std::string> message;
  };
  bool operator==(const HealthResult& a, const HealthResult& b);
  std::ostream& operator<<(std::ostream& stream,
                           const HealthResult& healthResult);
  /**
   * @brief Structure used/inherited in various methods of {@link ApiClient},
   * has the server's status.
   *
   */
  struct Response {
    Health health;
    std::optional<std::string> message;
  };
  /**
   * @brief
   * Structure used in {@link RoomStatus}'s devices, has the information of each
   * device connected to the room.
   *
   */
  struct Device {
    /** @brief An unique name for the device which is connected to the room,
     * such as collodi's fedora linux.*/
    std::string name;
    /** @brief Timestamp of when did this device connect to the room. */
    std::uint64_t timestamp;
  };
  /**
   * @brief Structure used as result for {@link getRoomStatus}, has the status
   * of the room and inherits {@link Response}.
   *
   */
  struct RoomStatus : Response {
    /** @brief Name of the room. */
    std::string name;
    /** @brief Information for all of the devices connected to the room. */
    std::vector<Device> devices;
    /** @brief Id of the room. */
    std::uint64_t id;
  };
  bool operator==(const RoomStatus& a, const RoomStatus& b);
  std::ostream& operator<<(std::ostream& stream, const RoomStatus& roomStatus);
  /**
   * @brief Enum used in {@link ContentStatus}, represents the type of {@link
   * Content}
   *
   */
  enum struct ContentType {
    /** @brief Type of {@link Content} is file. */
    File,
    /** @brief Type of {@link Content} is file. */
    Clipboard,
  };
  /**
   * @brief Structure used in {@link Content}, has the
   * status of {@link Content}.
   *
   */
  struct ContentStatus {
    /** @brief Device which uploaded this content.*/
    std::string device;
    /** @brief Timestamp of when did this content get uploaded.*/
    std::uint64_t timestamp;
    /** @brief Type of {@link Content}.*/
    ContentType type;
    /** @brief Optional: If the type is file, then this represents it's name.*/
    std::optional<std::string> name;
    /** @brief MIME.*/
    std::string mime;
  };
  /**
   * @brief Structure used as result for {@link getContent}, has data and {@link
   * ContentStatus} and inherits {@link Response}.
   *
   */
  struct Content : Response {
    /** @brief The status of {@link Content}.*/
    ContentStatus contentStatus;
    /** @brief The data of {@link Content}, is a variant of string and binary.*/
    std::variant<std::string, std::vector<std::uint8_t>> data;
  };
  /**
   * @brief Structure used as result for {@link createRoom}, has the room id and
   * inherits {@link Response}.
   *
   */
  struct RoomId : Response {
    std::uint64_t id;
  };
  bool operator==(const RoomId& a, const RoomId& b);
  std::ostream& operator<<(std::ostream& stream, const RoomId& roomId);
};     // namespace octane
#endif // OCTANE_API_CLIENT_API_RESULT_TYPES_H_