/**
 * @file api_result_types.h
 * @author soon (kento.soon@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-09-01
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef OCTANE_API_CLIENT_API_RESULT_TYPES_H_
#define OCTANE_API_CLIENT_API_RESULT_TYPES_H_

#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>
#include <ostream>

namespace octane {
  /**
   * @brief {@link HealthResult}において、{@link
   * Health}の取り得る値を表す列挙体。
   *
   */
  enum struct Health {
    /** @brief サーバーが正常に動いている時の戻り値。*/
    Healthy,
    /** @brief サーバーに障害が発生している時の戻り値。*/
    Degraded,
    /** @brief サーバーが使用不能である時の戻り値。*/
    Faulty,
  };
  /**
   * @brief {@link health}メソッドを呼んだ時の結果を表す構造体。
   *
   */
  struct HealthResult {
    /** @brief サーバーの状態。 */
    Health health;
    /** @brief ユーザに詳細を通知するメッセージ。 */
    std::optional<std::string> message;
  };
  bool operator==(const HealthResult& a, const HealthResult& b);
  std::ostream& operator<<(std::ostream& stream, const HealthResult& healthResult);
  /**
   * @brief
   * {@link RoomStatus}において{@link
   * std::vector<Device>}の要素が取り得る値を表す列挙体。
   *
   */
  struct Device {
    /** @brief デバイス名。 */
    std::string name;
    /** @brief タイムスタンプ。*/
    std::uint64_t timestamp;
  };
  /**
   * @brief {@link getRoomStatus}メソッドを呼んだ時の結果を表す構造体。
   *
   */
  struct RoomStatus {
    /** @brief ルームの名前。 */
    std::string name;
    /** @brief ルームに接続している全てのデバイス。 */
    std::vector<Device> devices;
  };
  /**
   * @brief {@link
   * Content}の型がファイルなのか、クリップボードなのかを表す列挙体。
   *
   */
  enum struct ContentType {
    /** @brief {@link Content}の型がファイルであることを表す。 */
    File,
    /** @brief {@link Content}の型がクリップボードであることを表す。 */
    Clipboard,
  };
  /**
   * @brief {@link getContentStatus}メソッドを呼んだ時の結果を表す構造体。
   *
   */
  struct ContentStatus {
    /** @brief デバイス名。*/
    std::string device;
    /** @brief タイムスタンプ。*/
    std::uint64_t timestamp;
    /** @brief コンテンツの型。*/
    ContentType type;
    /** @brief コンテンツがファイル形式をとる場合、その名前。*/
    std::optional<std::string> name;
    /** @brief MIME。*/
    std::string mime;
  };
  /**
   * @brief {@link getContent}メソッドを呼んだ時の結果を表す構造体。
   *
   */
  struct Content {
    /** @brief コンテンツの状態。*/
    ContentStatus contentStatus;
    /** @brief コンテンツのデータ。*/
    std::variant<std::string, std::vector<std::uint8_t>> data;
  };
};     // namespace octane
#endif // OCTANE_API_CLIENT_API_RESULT_TYPES_H_