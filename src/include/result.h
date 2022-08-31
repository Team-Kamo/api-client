#ifndef OCTANE_API_CLIENT_RESULT_H_
#define OCTANE_API_CLIENT_RESULT_H_

#include <cassert>
#include <utility>

namespace octane {
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
    operator bool() const noexcept {
      assert(tag != Tag::None);
      return tag == Tag::OK;
    }
    bool operator!() const noexcept {
      assert(tag != Tag::None);
      return tag == Tag::Error;
    }
    const T_OK& get() const {
      assert(tag == Tag::OK);
      return ok;
    }
    T_OK& get() {
      assert(tag == Tag::OK);
      return ok;
    }
    const T_Error& err() const {
      assert(tag == Tag::Error);
      return error;
    }
    T_Error& err() {
      assert(tag == Tag::Error);
      return error;
    }
  };

  struct _ {};

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
  template <typename T_OK>
  decltype(auto) ok(const T_OK& ok) {
    return ok_t(ok);
  }
  template <typename T_OK>
  decltype(auto) ok(T_OK&& ok) {
    return ok_t(std::move(ok));
  }
  template <typename T_OK = _>
  ok_t<_> ok() {
    return ok(_{});
  }

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
  template <typename T_Error, size_t N>
  decltype(auto) error(const T_Error (&error)[N]) {
    return error_t((const T_Error*)error);
  }
  template <typename T_Error>
  decltype(auto)  error(const T_Error& error) {
    return error_t(error);
  }
  template <typename T_Error>
  decltype(auto) error(T_Error&& error) {
    return error_t(std::move(error));
  }
} // namespace octane

#endif // OCTANE_API_CLIENT_RESULT_H_