/**
 * @file api_result_types.cpp
 * @author soon (kento.soon@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-09-02
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "include/api_result_types.h"

namespace octane {
  bool operator==(const HealthResult& a, const HealthResult& b) {
    return (a.health == b.health && a.message == b.message);
  }
  std::ostream& operator<<(std::ostream& stream,
                           const HealthResult& healthResult) {
    std::string health;
    if (healthResult.health == Health::Healthy) {
      health = "healthy";
    } else if (healthResult.health == Health::Degraded) {
      health = "degraded";
    } else if (healthResult.health == Health::Faulty) {
      health = "faulty";
    } else {
      health = "unknown";
    }
    stream << "health = " << health
           << ", message = " << healthResult.message.value_or("<nullopt>");
    return stream;
  };
  bool operator==(const RoomId& a, const RoomId& b) {
    return (a.id == b.id);
  }
  std::ostream& operator<<(std::ostream& stream, const RoomId& roomId) {
    stream << "id = " << roomId.id;
    return stream;
  };
  bool operator==(const RoomStatus& a, const RoomStatus& b) {
    bool isEqual = true;
    if (a.name != b.name) isEqual = false;
    if (a.devices.size() != b.devices.size()) isEqual = false;
    for (int i = 0; i < a.devices.size(); i++) {
      if (a.devices[i].name != b.devices[i].name) isEqual = false;
      if (a.devices[i].timestamp != b.devices[i].timestamp) isEqual = false;
    }
    return isEqual;
  }
  std::ostream& operator<<(std::ostream& stream, const RoomStatus& roomStatus) {
    stream << "name = " << roomStatus.name << ", devices = [";
    for (const auto& device : roomStatus.devices) {
      stream << "{ name = " << device.name
             << " timestamp = " << device.timestamp;
    }
    return stream;
  };
  bool operator==(const ContentStatus& a, const ContentStatus& b) {
    if (a.device != b.device) return false;
    if (a.mime != b.mime) return false;
    if (a.name != b.name) return false;
    if (!(int(a.timestamp) == int(b.timestamp))) return false;
    if (!(a.type == b.type)) return false;
    return true;
  }
  std::ostream& operator<<(std::ostream& stream,
                           const ContentStatus& contentStatus) {
    std::string type;
    if (contentStatus.type == ContentType::Clipboard) {
      type = "clipboard";
    } else if (contentStatus.type == ContentType::File) {
      type = "file";
    }
    stream << "device = " << contentStatus.device
           << " mime = " << contentStatus.mime
           << " name = " << contentStatus.name.value_or("")
           << " timestamp = " << contentStatus.timestamp << " type = " << type;
    return stream;
  };
} // namespace octane