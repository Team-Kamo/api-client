# Octane API Client

OctaneのAPIサーバとの通信を仲介し、後続のクライアント実装へ提供するための中間ライブラリ。

APIとの煩雑なやりとりを隠蔽し、クライアント実装に必要な機能を提供することを目的に作られた。

例えばAPIの制約上必要である30分に一回のヘルスチェックや連続して呼ぶ必要のあるAPIリクエストを自動的に呼び出すなどの機能を持つ。

## APIリファレンス

ドキュメントは[こちら](https://team-kamo.github.io/api-client/)。信頼と実績のDoxygen製。

## ビルド&テスト方法

各環境向けにスクリプト書いたのでそれ使ってください。

- [cmd](https://github.com/Team-Kamo/api-client/blob/master/build.bat)
- [PowerShell](https://github.com/Team-Kamo/api-client/blob/master/build.ps1)
- [bashなど](https://github.com/Team-Kamo/api-client/blob/master/build.sh)

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
