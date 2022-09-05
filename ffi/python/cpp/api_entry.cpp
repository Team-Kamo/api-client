#define NOMINMAX
#include <Windows.h>
#include <include/api_client.h>
#include <include/error_code.h>

#if defined(_MSC_VER)
#define OCTANE_API __declspec(dllexport)
#elif defined(__GNUC__)
#define OCTANCE_API __attribute__((visibility("default")))
#else
#define OCTANE_API
#endif

extern "C" {
struct OctaneApiClientErrorStructure {
  char* code;
  char* reason;
};
struct OctaneApiClientRootStructure {
  octane::ApiClient client;
};
struct OctaneApiClientDeviceStructure {
  char* name;
  std::uint64_t timestamp;
};
struct OctaneApiClientRoomStatusStructure {
  char* name;
  std::uint64_t id;
  std::uint64_t num_devices;
  OctaneApiClientDeviceStructure* devices;
};
struct OctaneApiClientByteArray {
  std::uint64_t size;
  void* data;
};
struct OctaneApiClientFileInfoStructure {
  char* filename;
  OctaneApiClientByteArray data;
};
struct OctaneApiClientMultiFileStructure {
  std::uint64_t num_files;
  OctaneApiClientFileInfoStructure* files;
};
union OctaneApiClientContentData {
  OctaneApiClientByteArray file;
  char* clipboard;
  OctaneApiClientMultiFileStructure multi_file;
};
struct OctaneApiClientContentStructure {
  char* device;
  std::uint64_t timestamp;
  std::int32_t type;
  char* name;
  char* mime;
  OctaneApiClientContentData data;
};

// ゼロ終端を追加する
static char* strToCharArray(std::string_view str) {
  auto cstr = new char[str.size() + 1];
  memcpy(cstr, str.data(), str.size());
  cstr[str.size()] = '\0';
  return cstr;
}
static char* vecToCharArray(const std::vector<std::uint8_t>& data) {
  auto cstr = new char[data.size() + 1];
  memcpy(cstr, data.data(), data.size());
  cstr[data.size()] = '\0';
  return cstr;
}
// ゼロ終端を追加しない
static OctaneApiClientByteArray toByteArray(
  const std::vector<std::uint8_t>& data) {
  auto bytes = new std::uint8_t[data.size()];
  memcpy(bytes, data.data(), data.size());
  return OctaneApiClientByteArray{
    .size = data.size(),
    .data = bytes,
  };
}
static std::vector<uint8_t> fromByteArray(
  const OctaneApiClientByteArray& byteArray) {
  return std::vector<uint8_t>((std::uint8_t*)byteArray.data,
                              (std::uint8_t*)byteArray.data + byteArray.size);
}

static OctaneApiClientErrorStructure octane_api_client__last_error{};
void makeError(const octane::ErrorResponse& err) {
  delete[] octane_api_client__last_error.code;
  delete[] octane_api_client__last_error.reason;

  octane_api_client__last_error.code   = strToCharArray(err.code);
  octane_api_client__last_error.reason = strToCharArray(err.reason);
}
static void clearError() {
  delete[] octane_api_client__last_error.code;
  delete[] octane_api_client__last_error.reason;

  octane_api_client__last_error.code   = strToCharArray(octane::ERR_API_CLIENT_OK);
  octane_api_client__last_error.reason = strToCharArray("");
}

OCTANE_API OctaneApiClientErrorStructure octane_api_client__get_last_error() {
  return octane_api_client__last_error;
}

/**
 * @brief ApiClientを初期化する。
 * @details
 * 使用者はライブラリ使用時にはじめにこれを呼び出さなければならない。
 * また、この関数の戻り値は他の関数を呼び出すときに使用するため補完すること。
 * もしnullptrが返ってきた場合は初期化に失敗しているので{@link
 * octane_api_client__get_last_error}を呼び出して詳細を取得すること。
 *
 * @param token [c_char_p] APIのトークン
 * @param origin [c_char_p] APIサーバのオリジン
 * @param baseUrl [c_char_p] APIサーバのベースURL
 * @return void* [c_void_p]
 * 今後のAPI呼び出しに必要な情報。nullptrの場合は関数の実行に失敗している。
 */
OCTANE_API void* octane_api_client__init(char* token,
                                         char* origin,
                                         char* baseUrl) {
  auto obj = new OctaneApiClientRootStructure{
    .client = octane::ApiClient(token, origin, baseUrl),
  };
  auto result = obj->client.init();
  if (!result) {
    makeError(result.err());
    delete obj;
    return nullptr;
  }
  clearError();
  return obj;
}
/**
 * @brief 終了処理。アプリケーション終了時に"絶対"に呼び出すこと。
 *
 * @return OCTANE_API API
 */
OCTANE_API void octane_api_client__destroy(void* api) {
  auto obj = (OctaneApiClientRootStructure*)api;
  delete obj;
  delete[] octane_api_client__last_error.code;
  delete[] octane_api_client__last_error.reason;
}

/**
 * @brief ルームを作成する。
 * 失敗した場合は0を返すため、{@link
 * octane_api_client__get_last_error}を呼び出して詳細を取得すること。
 *
 * @param api [c_void_p] API
 * @param roomName [c_char_p] 作成するルーム名
 * @return std::uint64_t [c_ulonglong] ルームID
 */
OCTANE_API std::uint64_t octane_api_client__create_room(void* api,
                                                        char* roomName) {
  clearError();
  auto obj    = (OctaneApiClientRootStructure*)api;
  auto result = obj->client.createRoom(roomName);
  if (!result) {
    makeError(result.err());
    return 0;
  }
  return result.get().id;
}

OCTANE_API bool octane_api_client__connect_room(void* api,
                                                std::uint64_t id,
                                                char* name) {
  clearError();
  auto obj    = (OctaneApiClientRootStructure*)api;
  auto result = obj->client.connectRoom(id, name);
  if (!result) {
    makeError(result.err());
    return false;
  }
  return true;
}

OCTANE_API bool octane_api_client__disconnect_room(void* api,
                                                   std::uint64_t id,
                                                   char* name) {
  clearError();
  auto obj    = (OctaneApiClientRootStructure*)api;
  auto result = obj->client.disconnectRoom(id, name);
  if (!result) {
    makeError(result.err());
    return false;
  }
  return true;
}

/**
 * @brief
 * @details
 * 情報が不要になったら{@link
 * octane_api_client__free_room_status_structure}でオブジェクトを削除すること。
 *
 * @param api
 * @param id
 * @return OCTANE_API*
 */
OCTANE_API OctaneApiClientRoomStatusStructure*
octane_api_client__get_room_status(void* api, std::uint64_t id) {
  clearError();
  auto obj    = (OctaneApiClientRootStructure*)api;
  auto result = obj->client.getRoomStatus(id);
  if (!result) {
    makeError(result.err());
    return nullptr;
  }

  auto status         = new OctaneApiClientRoomStatusStructure();
  status->name        = strToCharArray(result.get().name);
  status->id          = result.get().id;
  status->num_devices = result.get().devices.size();
  status->devices     = new OctaneApiClientDeviceStructure[status->num_devices];
  for (size_t i = 0; i < status->num_devices; ++i) {
    auto& device                 = result.get().devices[i];
    status->devices[i].name      = strToCharArray(device.name);
    status->devices[i].timestamp = device.timestamp;
  }
  return status;
}

OCTANE_API void octane_api_client__free_room_status_structure(
  OctaneApiClientRoomStatusStructure* status) {
  clearError();
  for (size_t i = 0; i < status->num_devices; ++i) {
    delete[] status->devices[i].name;
  }
  delete status;
}

OCTANE_API bool octane_api_client__delete_room(void* api, std::uint64_t id) {
  clearError();
  auto obj    = (OctaneApiClientRootStructure*)api;
  auto result = obj->client.deleteRoom(id);
  if (!result) {
    makeError(result.err());
    return false;
  }
  return true;
}

OCTANE_API OctaneApiClientContentStructure* octane_api_client__get_content(
  void* api) {
  clearError();
  auto obj    = (OctaneApiClientRootStructure*)api;
  auto result = obj->client.getContent();
  if (!result) {
    makeError(result.err());
    return nullptr;
  }

  auto& status       = result.get().contentStatus;
  auto content       = new OctaneApiClientContentStructure();
  content->device    = strToCharArray(status.device);
  content->timestamp = status.timestamp;
  content->type      = (std::int32_t)status.type;
  content->name      = strToCharArray(status.name.value_or(""));
  content->mime      = strToCharArray(status.mime);

  switch (status.type) {
    case octane::ContentType::File:

      content->data.file
        = toByteArray(std::get<std::vector<std::uint8_t>>(result.get().data));
      break;
    case octane::ContentType::Clipboard:
      content->data.clipboard
        = vecToCharArray(std::get<std::vector<std::uint8_t>>(result.get().data));
      break;
    case octane::ContentType::MultiFile:
      auto& files = std::get<std::vector<octane::FileInfo>>(result.get().data);
      content->data.multi_file.num_files = files.size();
      content->data.multi_file.files
        = new OctaneApiClientFileInfoStructure[content->data.multi_file
                                                 .num_files];
      for (size_t i = 0; i < content->data.multi_file.num_files; ++i) {
        content->data.multi_file.files[i].filename
          = strToCharArray(files[i].filename);
        content->data.multi_file.files[i].data = toByteArray(files[i].data);
      }
      break;
  }

  return content;
}

OCTANE_API void octane_api_client__free_content_structure(
  OctaneApiClientContentStructure* content) {
  delete[] content->device;
  delete[] content->name;
  delete[] content->mime;
  switch ((octane::ContentType)content->type) {
    case octane::ContentType::File:
      delete[] content->data.file.data;
      break;
    case octane::ContentType::Clipboard:
      delete[] content->data.clipboard;
      break;
    case octane::ContentType::MultiFile:
      for (size_t i = 0; i < content->data.multi_file.num_files; ++i) {
        delete[] content->data.multi_file.files[i].filename;
        delete[] content->data.multi_file.files[i].data.data;
      }
      break;
  }
}

OCTANE_API bool octane_api_client__delete_content(void* api) {
  clearError();
  auto obj    = (OctaneApiClientRootStructure*)api;
  auto result = obj->client.deleteContent();
  if (!result) {
    makeError(result.err());
    return false;
  }
}

OCTANE_API bool octane_api_client__upload_content(
  void* api,
  OctaneApiClientContentStructure* content) {
  clearError();
  auto obj = (OctaneApiClientRootStructure*)api;

  octane::Content ctn{
    .contentStatus = octane::ContentStatus{
      .device = content->device,
      .timestamp = content->timestamp,
      .type = (octane::ContentType)content->type,
      .name = content->name,
      .mime = content->mime,
    },
  };
  switch ((octane::ContentType)content->type) {
    case octane::ContentType::File:
      ctn.data = fromByteArray(content->data.file);
      break;
    case octane::ContentType::Clipboard:
      ctn.data = std::string(content->data.clipboard);
      break;
    case octane::ContentType::MultiFile:
      std::vector<octane::FileInfo> files;
      files.reserve(content->data.multi_file.num_files);
      for (size_t i = 0; i < content->data.multi_file.num_files; ++i) {
        files.emplace_back(
          content->data.multi_file.files[i].filename,
          fromByteArray(content->data.multi_file.files[i].data));
      }
      ctn.data = std::move(files);
      break;
  }

  auto result = obj->client.uploadContent(ctn);
  if (!result) {
    makeError(result.err());
    return false;
  }
  return true;
}
}