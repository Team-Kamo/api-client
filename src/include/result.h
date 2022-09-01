/**
 * @file result.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief RustのResultに触発されて作った結果を表す型を定義する。
 * @version 0.1
 * @date 2022-09-01
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef OCTANE_API_CLIENT_RESULT_H_
#define OCTANE_API_CLIENT_RESULT_H_

#include <cassert>
#include <utility>

namespace octane {
  /**
   * @brief 正常系と準正常系で示す値を分岐させるラッパークラス。
   * @details
   * このクラスはRustに触発されてこしらえました。
   * 実装雑なので与える型によってはコンパイル通らない可能性あり。
   *
   * @tparam T_OK 成功した時の型。
   * @tparam T_Error 失敗した時の型。
   *
   * @note
   * C++には便利な例外機構が存在するが、例外処理は面倒な上
   * オーバーヘッドも地味にあるので本ライブラリではこちらを使う。
   */
  template <typename T_OK, typename T_Error>
  class [[nodiscard]] Result {
    enum struct Tag { None, OK, Error };
    Tag tag;
    union {
      T_OK ok;
      T_Error error;
    };

    template <typename T>
    friend class ok_t;

    template <typename T>
    friend class error_t;

  public:
    Result(const Result& result) : tag(result.tag) {
      switch (tag) {
      case Tag::None:
        break;
      case Tag::OK:
        new (&ok) T_OK(result.ok);
        break;
      case Tag::Error:
        new (&error) T_Error(result.error);
        break;
      default:
        std::abort();
      }
    }
    Result(Result&& result) : tag(result.tag) {
      switch (tag) {
      case Tag::None:
        break;
      case Tag::OK:
        new (&ok) T_OK(std::move(result.ok));
        break;
      case Tag::Error:
        new (&error) T_Error(std::move(result.error));
        break;
      default:
        std::abort();
      }
      result.tag = Tag::None;
    }
    ~Result() {
      switch (tag) {
      case Tag::None:
        break;
      case Tag::OK:
        ok.~T_OK();
        break;
      case Tag::Error:
        error.~T_Error();
        break;
      default:
        std::abort();
      }
      tag = Tag::None;
    }

    Result& operator=(const Result& result)& {
      if (&result != this) {
        switch (result.tag) {
        case Tag::None:
          this->~Result();
          break;
        case Tag::OK:
          *this = result.ok();
          break;
        case Tag::Error:
          *this = result.error();
          break;
        default:
          std::abort();
        }
      }
      return *this;
    }
    Result& operator=(Result&& result)& {
      assert(this != &result);
      switch (result.tag) {
      case Tag::None:
        this->~Result();
        break;
      case Tag::OK:
        *this = std::move(result.ok);
        break;
      case Tag::Error:
        *this = std::move(result.error);
        break;
      default:
        std::abort();
      }
      result.tag = Tag::None;
      return *this;
    }

  private:
    Result(const T_OK& ok, [[maybe_unused]] int) : tag(Tag::OK), ok(ok) {}
    Result(T_OK&& ok, [[maybe_unused]] int) : tag(Tag::OK), ok(std::move(ok)) {}

    Result(const T_Error& error) : tag(Tag::Error), error(error) {}
    Result(T_Error&& error) : tag(Tag::Error), error(std::move(error)) {}

  public:
    /**
     * @brief 正常系であるかを判定する。
     *
     * @return true 正常系であるとき。
     * @return false 純正常系であるとき。
     */
    operator bool() const noexcept {
      assert(tag != Tag::None);
      return tag == Tag::OK;
    }
    /**
     * @brief 準正常系であるかを判定する。
     *
     * @return true 準正常系であるとき。
     * @return false 正常系であるとき。
     */
    bool operator!() const noexcept {
      assert(tag != Tag::None);
      return tag == Tag::Error;
    }
    /**
     * @brief 正常系の値を定数値として取り出す。
     * @details
     * このメソッドは必ず事前に正常系であることを確認してから呼び出さなければならない。
     *
     * @return const T_OK& 正常系の値。
     */
    const T_OK& get() const {
      assert(tag == Tag::OK);
      return ok;
    }
    /**
     * @brief 正常系の値を取り出す。
     * @details
     * このメソッドは必ず事前に正常系であることを確認してから呼び出さなければならない。
     *
     * @return T_OK& 正常系の値。
     */
    T_OK& get() {
      assert(tag == Tag::OK);
      return ok;
    }
    /**
     * @brief 準正常系の値を定数値として取り出す。
     * @details
     * このメソッドは事前に準正常系であることを確認してから呼び出さなければならない。
     *
     * @return const T_Error& 準正常系の値。
     */
    const T_Error& err() const {
      assert(tag == Tag::Error);
      return error;
    }
    /**
     * @brief 準正常系の値を取り出す。
     * @details
     * このメソッドは事前に準正常系であることを確認してから呼び出さなければならない。
     *
     * @return T_Error& 準正常系の値。
     */
    T_Error& err() {
      assert(tag == Tag::Error);
      return error;
    }
  };

  /**
   * @brief 何も返さないことを表す構造体。
   * @details
   * 例えば、正常系は何も値を返さないが準正常系はエラーメッセージを返したいという場合に
   * Result<_, std::string>とすることでそれを表現する。
   *
   */
  struct _ {};

  /**
   * @brief 正常系の型のみを規定したいときに使えるユーティリティクラス。
   * @details
   * {@link octane::ok}で内部的に用いられる型。
   * 任意の{@link Result}にキャストすることができる。
   *
   * @tparam T 正常系の型。
   */
  template <typename T>
  class ok_t {
    T ok;

  public:
    explicit ok_t(const T& ok) : ok(ok) {}
    explicit ok_t(T&& ok) : ok(std::move(ok)) {}
    template <typename T_OK, typename T_Error>
    operator Result<T_OK, T_Error>() {
      return Result<T_OK, T_Error>(std::move(ok), 0);
    }
  };
  /**
   * @brief 正常系の値を構築するためのユーティリティ関数。
   * @details
   * 例えば{@link Result}を返す関数で以下のように使用する。
   * @code {.cpp}
   * return ok("OK!");
   * @endcode
   *
   * @tparam T_OK 正常系の型。
   * @param[in] ok 正常系の値。
   * @return decltype(auto) 構築した正常系を示すオブジェクト。
   */
  template <typename T_OK>
  decltype(auto) ok(const T_OK& ok) {
    return ok_t(ok);
  }
  /**
   * @brief 正常系の値を構築するためのユーティリティ関数。
   * @details
   * 例えば{@link Result}を返す関数で以下のように使用する。
   * @code {.cpp}
   * return ok("OK!");
   * @endcode
   *
   * @tparam T_OK 正常系の型。
   * @param[in] ok 正常系の値。
   * @return decltype(auto) 構築した正常系を示すオブジェクト。
   */
  template <typename T_OK>
  decltype(auto) ok(T_OK&& ok) {
    return ok_t(std::move(ok));
  }
  /**
   * @brief 正常系の型が{@link _}であるときに使用できるユーティリティ関数。
   * @details
   * 例えば{@link Result}を返す関数で以下のように使用する。
   * @code {.cpp}
   * return ok();
   * @endcode

   * @tparam T_OK _
   * @return ok_t<_> 構築した正常系を示すオブジェクト。
   */
  template <typename T_OK = _>
  ok_t<_> ok() {
    return ok(_{});
  }

  /**
   * @brief 準正常系のみを規定したいときに使えるユーティリティクラス。
   * @details
   * {@link octane::error}で内部的に用いられる型。
   * 任意の{@link Result}にキャストすることができる。
   *
   * @tparam T 準正常系の型。
   */
  template <typename T>
  class error_t {
    T error;

  public:
    explicit error_t(const T& error) : error(error) {}
    explicit error_t(T&& error) : error(std::move(error)) {}
    template <typename T_OK, typename T_Error>
    operator Result<T_OK, T_Error>() {
      return Result<T_OK, T_Error>(std::move(error));
    }
  };
  /**
   * @brief 準正常系の値を構築するためのユーティリティ関数。
   * @details
   * このオーバーロードは正常系が固定長配列のときに使用される。
   * 例えば{@link Result}を返す関数で以下のように使用する。
   * @code {.cpp}
   * return error("Error!");
   * @endcode
   *
   * @tparam T_Error 準正常系の型。
   * @tparam N 配列の大きさ。
   * @param[in] error 準正常系の値。
   * @return decltype(auto) 構築した準正常系を示すオブジェクト。
   *
   * @note
   * このオーバーロードはエラーを文字列リテラルで表現したかったゆえに生まれた。
   * {@link ok}の方で実装していない理由は単に正常系で文字列リテラルを返すことがなかったから。
   * 需要があればそちらも作成する。
   */
  template <typename T_Error, size_t N>
  decltype(auto) error(const T_Error(&error)[N]) {
    return error_t((const T_Error*)error);
  }
  /**
   * @brief 準正常系の値を構築するためのユーティリティ関数。
   * @details
   * 例えば{@link Result}を返す関数で以下のように使用する。
   * @code {.cpp}
   * SomeError someError;
   * return error(someError);
   * @endcode
   *
   * @tparam T_Error 準正常系の型。
   * @param[in] error 準正常系の値。
   * @return decltype(auto) 構築した準正常系を示すオブジェクト。
   */
  template <typename T_Error>
  decltype(auto)  error(const T_Error& error) {
    return error_t(error);
  }
  /**
   * @brief 準正常系の値を構築するためのユーティリティ関数。
   * @details
   * 例えば{@link Result}を返す関数で以下のように使用する。
   * @code {.cpp}
   * SomeError someError;
   * return error(std::move(someError));
   * @endcode
   *
   * @tparam T_Error 準正常系の型。
   * @param[in] error 準正常系の値。
   * @return decltype(auto) 構築した準正常系を示すオブジェクト。
   */
  template <typename T_Error>
  decltype(auto) err(T_Error&& error) {
    return error_t(std::move(error));
  }
} // namespace octane

#endif // OCTANE_API_CLIENT_RESULT_H_