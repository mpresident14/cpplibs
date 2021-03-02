#ifndef PREZ_MISC_MOVABLE_FN_HPP
#define PREZ_MISC_MOVABLE_FN_HPP

#include <memory>
#include <type_traits>

namespace prez {
namespace misc {

namespace detail {
template <typename R, typename... Args>
class FnWrapper {
public:
  virtual ~FnWrapper() {}
  virtual R operator()(Args... args) = 0;
};

template <typename Fn, typename R, typename... Args>
class FnWrapperImpl : public FnWrapper<R, Args...> {
public:
  FnWrapperImpl(Fn&& fn) : fn_(std::forward<Fn>(fn)) {}

  FnWrapperImpl(const FnWrapperImpl&) = delete;
  FnWrapperImpl(FnWrapperImpl&&) = delete;
  FnWrapperImpl& operator=(const FnWrapperImpl&) = delete;
  FnWrapperImpl& operator=(FnWrapperImpl&&) = delete;

  R operator()(Args... args) override { return fn_(args...); }

private:
  Fn fn_;
};
} // namespace detail

/*
 * Quick and dirty version of std::function that accepts a lambda that isn't
 * copy-constructible (i.e. captures move-only objects)
 */
template <typename R, typename... Args>
class MovableFn;

template <typename R, typename... Args>
class MovableFn<R(Args...)> {
public:
  template <typename Fn>
  requires std::is_convertible_v<std::invoke_result_t<Fn, Args...>, R> MovableFn(Fn&& fn)
      : fnWrapper_(std::make_unique<detail::FnWrapperImpl<Fn, R, Args...>>(std::forward<Fn>(fn))) {}

  MovableFn(const MovableFn&) = delete;
  MovableFn(MovableFn&&) = default;
  MovableFn& operator=(const MovableFn&) = delete;
  MovableFn& operator=(MovableFn&&) = default;

  R operator()(Args... args) { return (*fnWrapper_)(args...); }

  template <typename R2, typename... Args2, typename Fn>
  friend MovableFn<R2(Args2...)> createMovableFn(Fn&& fn);

private:
  std::unique_ptr<detail::FnWrapper<R, Args...>> fnWrapper_;
};

} // namespace misc
} // namespace prez

#endif // MOVABLE_FN_HPP
