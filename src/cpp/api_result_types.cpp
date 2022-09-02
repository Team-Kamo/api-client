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
    stream << "health = " << health << ", message = " << healthResult.message.value_or("<nullopt>");
    return stream;
  };
} // namespace octane