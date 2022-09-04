/**
 * @file result.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief Define type to represent the result, inspired by Rust's "Result".
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
   * @brief Wrapper class that bifurcates the values indicated by normal and
   * quasi-normal states.
   * @details
   * このクラスはRustに触発されてこしらえました。
   * 実装雑なので与える型によってはコンパイル通らない可能性あり。
   * This class was inspired by Rust.
   * The implementation is crude, so it may not compile depending on the type
   * you give it.
   *
   * @tparam T_OK Type on success.
   * @tparam T_Error Type on error.
   *
   * @note
   * C++には便利な例外機構が存在するが、例外処理は面倒な上
   * オーバーヘッドも地味にあるので本ライブラリではこちらを使う。
   * C++ has a convenient exception mechanism, but exception handling is tedious
   * and there is an overhead, so this library uses this one.
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

    Result& operator=(const Result& result) & {
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
    Result& operator=(Result&& result) & {
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
     * @brief Judges whether the state is normal.
     *
     * @return true When the state is normal.
     * @return false When the state is quasi-normal.
     */
    operator bool() const noexcept {
      assert(tag != Tag::None);
      return tag == Tag::OK;
    }
    /**
     * @brief Judges whether the state is quasi-normal.
     *
     * @return true When the state is quasi-normal.
     * @return false When the state is normal.
     */
    bool operator!() const noexcept {
      assert(tag != Tag::None);
      return tag == Tag::Error;
    }
    /**
     * @brief Get the value of normal state as a constant.
     * @details
     * このメソッドは必ず事前に正常系であることを確認してから呼び出さなければならない。
     * This method must be called after checking that the state is normal.
     *
     * @return const T_OK& Value of normal state.
     */
    const T_OK& get() const {
      assert(tag == Tag::OK);
      return ok;
    }
    /**
     * @brief Get the value of normal state.
     * @details
     * このメソッドは必ず事前に正常系であることを確認してから呼び出さなければならない。
     * This method must be called after checking that the state is normal.
     *
     * @return T_OK& Value of normal state.
     */
    T_OK& get() {
      assert(tag == Tag::OK);
      return ok;
    }
    /**
     * @brief Get the value of quasi-normal state as a constant.
     * @details
     * このメソッドは事前に準正常系であることを確認してから呼び出さなければならない。
     * This method must be called after checking that the state is quasi-normal.
     *
     * @return const T_Error& Value of quasi-normal state.
     */
    const T_Error& err() const {
      assert(tag == Tag::Error);
      return error;
    }
    /**
     * @brief Get the value of quasi-normal state.
     * @details
     * このメソッドは事前に準正常系であることを確認してから呼び出さなければならない。
     * This method must be called after checking that the state is quasi-normal.
     *
     * @return T_Error& Value of quasi-normal state.
     */
    T_Error& err() {
      assert(tag == Tag::Error);
      return error;
    }
  };

  /**
   * @brief Structure representing that it returns nothing.
   * @details
   * 例えば、正常系は何も値を返さないが準正常系はエラーメッセージを返したいという場合に
   * Result<_, std::string>とすることでそれを表現する。
   * For example, if you want a normal state to return no value but a
   * quasi-normal state to return an error message, use Result<_, std::string>.
   *
   */
  struct _ {};

  /**
   * @brief Utility class that can be used when we only want to specify normal
   * state types.
   * @details
   * {@link octane::ok}で内部的に用いられる型。
   * 任意の{@link Result}にキャストすることができる。
   * Types used internally in {@link octane::ok}.
   * Can be cast to any {@link Result}.
   *
   * @tparam T Type of normal state.
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
   * @brief Utility function used to construct normal state values.
   * @details
   * 例えば{@link Result}を返す関数で以下のように使用する。
   * Use it as following in a function that returns {@link Result}.
   * @code {.cpp}
   * return ok("OK!");
   * @endcode
   *
   * @tparam T_OK Type of normal state.
   * @param[in] ok Value of normal state.
   * @return decltype(auto) Object indicating the constructed normal state.
   */
  template <typename T_OK>
  decltype(auto) ok(const T_OK& ok) {
    return ok_t(ok);
  }
  /**
   * @brief Utility function used to construct normal state values.
   * @details
   * 例えば{@link Result}を返す関数で以下のように使用する。
   * Use it as following in a function that returns {@link Result}.
   * @code {.cpp}
   * return ok("OK!");
   * @endcode
   *
   * @tparam T_OK Type of normal state.
   * @param[in] ok Value of normal state.
   * @return decltype(auto) Object indicating the constructed normal state.
   */
  template <typename T_OK>
  decltype(auto) ok(T_OK&& ok) {
    return ok_t(std::move(ok));
  }
  /**
   * @brief Utitlity function used when the normal state type is {@link _}
   * @details
   * 例えば{@link Result}を返す関数で以下のように使用する。
   * Use it as following in a function that returns {@link Result}.
   * @code {.cpp}
   * return ok();
   * @endcode

   * @tparam T_OK _
   * @return ok_t<_> Object indicating the constructed normal state.
   */
  template <typename T_OK = _>
  ok_t<_> ok() {
    return ok(_{});
  }

  /**
   * @brief Utility class that can be used when we only want to specify
   * quasi-normal state types.
   * @details
   * {@link octane::error}で内部的に用いられる型。
   * 任意の{@link Result}にキャストすることができる。
   * Types used internally in {@link octane::error}.
   * Can be cast to any {@link Result}.
   *
   * @tparam T Type of quasi-normal state.
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
   * @brief Utility function used to construct quasi-normal state values.
   * @details
   * このオーバーロードは正常系が固定長配列のときに使用される。
   * 例えば{@link Result}を返す関数で以下のように使用する。
   * This overload is used when the normal state is a fixed length array.
   * Use it as following in a function that returns {@link Result}.
   * @code {.cpp}
   * return error("Error!");
   * @endcode
   *
   * @tparam T_Error Type of quasi-normal state.
   * @tparam N Size of array.
   * @param[in] error Value of quasi-normal state.
   * @return decltype(auto) Object indicating the constructed quasi-normal
   * state.
   *
   * @note
   * このオーバーロードはエラーを文字列リテラルで表現したかったゆえに生まれた。
   * {@link
   * ok}の方で実装していない理由は単に正常系で文字列リテラルを返すことがなかったから。
   * 需要があればそちらも作成する。
   * This overload was created because we wanted to express errors in string
   * literals.
   * The reason it was not implemented in {@link ok} was simply because the
   * normal system never returned a string literal. If there is a demand for it,
   * we will create it too.
   */
  template <typename T_Error, size_t N>
  decltype(auto) error(const T_Error (&error)[N]) {
    return error_t((const T_Error*)error);
  }
  /**
   * @brief Utility function to construct quasi-normal state values.
   * @details
   * 例えば{@link Result}を返す関数で以下のように使用する。
   * Use it as following in a function that returns {@link Result}.
   * @code {.cpp}
   * SomeError someError;
   * return error(someError);
   * @endcode
   *
   * @tparam T_Error Type of quasi-normal state.
   * @param[in] error Value of quasi-normal state.
   * @return decltype(auto) Object indicating the constructed quasi-normal
   * state.
   */
  template <typename T_Error>
  decltype(auto) error(const T_Error& error) {
    return error_t(error);
  }
  /**
   * @brief Utility function to construct quasi-normal state values.
   * @details
   * 例えば{@link Result}を返す関数で以下のように使用する。
   * Use it as following in a function that returns {@link Result}.
   * @code {.cpp}
   * SomeError someError;
   * return error(std::move(someError));
   * @endcode
   *
   * @tparam T_Error Type of quasi-normal state.
   * @param[in] error Value of quasi-normal state.
   * @return decltype(auto) Object indicating the constructed quasi-normal
   * state.
   */
  template <typename T_Error>
  decltype(auto) err(T_Error&& error) {
    return error_t(std::move(error));
  }
} // namespace octane

#endif // OCTANE_API_CLIENT_RESULT_H_