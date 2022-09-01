# Octane API Client

curl経由でAPIとの中継を担うライブラリ。

GUI、CUI共にこのライブラリからAPIへアクセスする。

## 仕様

ドキュメントは[こちら](./docs/index.html)。信頼と実績のDoxygen製。

## ビルド&テスト方法

各環境向けにスクリプト書いたのでそれ使ってください。

- [cmd](./build.bat)
- [PowerShell](./build.ps1)
- [bashなど](./build.sh)

## Git Submoduleでの使い方

gitにsubmoduleを追加する。

```sh
git submodule add https://github.com/Team-Kamo/api-client.git
```

ルートの`CMakeLists.txt`に次の項目を追加する。

```cmake
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/api-client/src/include)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/api-client)
```

ライブラリを使用するターゲットに`octane_api_client`を追加する

```cmake
target_link_libraries(your_target [PUBLIC, PRIVATE, INTERFACE] octane_api_client)
```
