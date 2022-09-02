#ifndef OCTANE_API_CLIENT_API_SCHEMA_H_
#define OCTANE_API_CLIENT_API_SCHEMA_H_

namespace octane::internal {
  constexpr auto SCHEMA_ERROR_RESPONSE        = R"(
    {
      "$schema": "https://json-schema.org/draft/2020-12/schema",
      "type": "object",
      "properties": {
        "code": {
          "type": "string",
          "title": "code"
        },
        "reason": {
          "type": "string",
          "title": "reason"
        }
      },
      "required": ["code", "reason"]
    }
  )";
  constexpr auto SCHEMA_HEALTH_GET            = R"(
    {
      "$schema": "https://json-schema.org/draft/2020-12/schema",
      "type": "object",
      "properties": {
        "health": {
          "enum": ["degraded", "faulty", "healthy"],
          "type": "string",
          "title": "health"
        },
        "message": {
          "type": "string",
          "title": "message"
        }
      },
      "required": ["health"]
    }
  )";
  constexpr auto SCHEMA_ROOM_POST             = R"(
    {
      "$schema": "https://json-schema.org/draft/2020-12/schema",
      "type": "object",
      "properties": {
        "id": {
          "type": "string",
          "title": "id"
        }
      },
      "required": ["id"]
    }
  )";
  constexpr auto SCHEMA_ROOM_ID_GET           = R"(
    {
      "$schema": "https://json-schema.org/draft/2020-12/schema",
      "type": "object",
      "properties": {
        "name": {
          "type": "string",
          "title": "name"
        },
        "devices": {
          "type": "array",
          "items": {
            "type": "object",
            "properties": {
              "name": {
                "type": "string",
                "title": "name"
              },
              "timestamp": {
                "type": "number",
                "title": "timestamp"
              }
            },
            "required": ["name", "timestamp"]
          },
          "title": "devices"
        }
      },
      "required": ["devices", "name"]
    }
  )";
  constexpr auto SCHEMA_ROOM_ID_DELETE        = R"(
    {
      "$schema": "https://json-schema.org/draft/2020-12/schema",
      "type": "object",
      "properties": {}
    }
  )";
  constexpr auto SCHEMA_ROOM_ID_POST          = R"(
    {
      "$schema": "https://json-schema.org/draft/2020-12/schema",
      "type": "object",
      "properties": {}
    }
  )";
  constexpr auto SCHEMA_ROOM_ID_STATUS_GET    = R"(
    {
      "$schema": "https://json-schema.org/draft/2020-12/schema",
      "type": "object",
      "properties": {
        "device": {
          "type": "string",
          "title": "device"
        },
        "timestamp": {
          "type": "number",
          "title": "timestamp"
        },
        "type": {
          "enum": ["clipboard", "file"],
          "type": "string",
          "title": "type"
        },
        "name": {
          "type": "string",
          "title": "name"
        },
        "mime": {
          "type": "string",
          "title": "mime"
        }
      },
      "required": ["device", "mime", "timestamp", "type"]
    }
  )";
  constexpr auto SCHEMA_ROOM_ID_STATUS_PUT    = R"(
    {
      "$schema": "https://json-schema.org/draft/2020-12/schema",
      "type": "object",
      "properties": {
        "device": {
          "type": "string",
          "title": "device"
        },
        "timestamp": {
          "type": "number",
          "title": "timestamp"
        },
        "type": {
          "enum": ["clipboard", "file"],
          "type": "string",
          "title": "type"
        },
        "name": {
          "type": "string",
          "title": "name"
        },
        "mime": {
          "type": "string",
          "title": "mime"
        }
      },
      "required": ["device", "mime", "timestamp", "type"]
    }
  )";
  constexpr auto SCHEMA_ROOM_ID_STATUS_DELETE = R"(
    {
      "$schema": "https://json-schema.org/draft/2020-12/schema",
      "type": "object",
      "properties": {}
    }
  )";
} // namespace octane::internal

#endif // OCTANE_API_CLIENT_API_SCHEMA_H_