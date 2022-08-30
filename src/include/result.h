#ifndef OCTANE_RESULT_H_
#define OCTANE_RESULT_H_

#include <cassert>
#include <utility>

namespace octane {
  template <typename OK, typename Error>
  class [[nodiscard]] Result {
    enum struct Tag { None, OK, Error };
    Tag tag;
    union {
      OK ok;
      Error error;
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
          new (&ok) OK(result.ok);
          break;
        case Tag::Error:
          new (&error) Error(result.error);
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
          new (&ok) OK(std::move(result.ok));
          break;
        case Tag::Error:
          new (&error) Error(std::move(result.error));
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
          ok.~OK();
          break;
        case Tag::Error:
          error.~Error();
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
    Result(const OK& ok, [[maybe_unused]] int) : tag(Tag::OK), ok(ok) {}
    Result(OK&& ok, [[maybe_unused]] int) : tag(Tag::OK), ok(std::move(ok)) {}

    Result(const Error& error) : tag(Tag::Error), error(error) {}
    Result(Error&& error) : tag(Tag::Error), error(std::move(error)) {}

  public:
    operator bool() const noexcept {
      assert(tag != Tag::None);
      return tag == Tag::OK;
    }
    bool operator!() const noexcept {
      assert(tag != Tag::None);
      return tag == Tag::Error;
    }
    const OK& get() const {
      assert(tag == Tag::OK);
      return ok;
    }
    OK& get() {
      assert(tag == Tag::OK);
      return ok;
    }
    const Error& err() const {
      assert(tag == Tag::Error);
      return error;
    }
    Error& err() {
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
    template <typename OK, typename Error>
    operator Result<OK, Error>() {
      return Result<OK, Error>(std::move(ok), 0);
    }
  };
  template <typename OK>
  decltype(auto) [[nodiscard]] ok(const OK& ok) {
    return ok_t(ok);
  }
  template <typename OK>
  decltype(auto) [[nodiscard]] ok(OK&& ok) {
    return ok_t(std::move(ok));
  }
  template <typename OK = _>
  ok_t<_> [[nodiscard]] ok() {
    return ok(_{});
  }

  template <typename T>
  class error_t {
    T error;

  public:
    explicit error_t(const T& error) : error(error) {}
    explicit error_t(T&& error) : error(std::move(error)) {}
    template <typename OK, typename Error>
    operator Result<OK, Error>() {
      return Result<OK, Error>(std::move(error));
    }
  };
  template <typename Error, size_t N>
  decltype(auto) [[nodiscard]] error(const Error (&error)[N]) {
    return error_t((const Error*)error);
  }
  template <typename Error>
  decltype(auto) [[nodiscard]] error(const Error& error) {
    return error_t(error);
  }
  template <typename Error>
  decltype(auto) [[nodiscard]] error(Error&& error) {
    return error_t(std::move(error));
  }
} // namespace octane

#endif // OCTANE_RESULT_H_