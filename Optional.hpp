#ifndef OPTIONAL_HPP
#define OPTIONAL_HPP
#include <cstdint>
#include <type_traits>
#include <iostream>
#include <new>
#include <utility>
#include <memory>
#include <concepts>
#include <stdexcept>

#if defined(__clang__) || defined(__GNUC__)
#  define ALWAYS_INLINE __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
#  define ALWAYS_INLINE __forceinline inline
#else
#  define ALWAYS_INLINE
#  warning Unknown compiler!
#  warning Expected Clang, GCC, or MSVC.
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The "null" type, similar to std::nullopt.

constexpr inline struct __Nullopt {
  constexpr __Nullopt() = default;
} Nullopt;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The Optional class itself.

template<class T>
class [[nodiscard]] Optional {
static_assert(!std::is_reference_v<T>);
static_assert(!std::is_array_v<T>);
public:
  using ValueType     = T;
  using ReferenceType = T&;
  using PointerType   = T*;

  [[nodiscard]] ALWAYS_INLINE auto value() & -> ReferenceType {
    _throw_if_inactive();
    return *std::launder<T>( reinterpret_cast<T*>(&value_) );
  }

  [[nodiscard]] ALWAYS_INLINE auto value() const& -> const T& {
    _throw_if_inactive();
    return *std::launder<const T>( reinterpret_cast<const T*>(&value_) );
  }

  [[nodiscard]] ALWAYS_INLINE auto value() && -> ValueType {
    return release_value(); // For rvalues, release ownership.
  }

  [[nodiscard]] ALWAYS_INLINE auto value_or(this auto&& self, T&& val) -> ValueType {
    if(self.has_value_)
      return std::forward<decltype(self)>(self).value();
    return val;
  }

  ALWAYS_INLINE auto operator->(this auto&& self) -> decltype(auto) {
    self._throw_if_inactive();
    return &( std::forward<decltype(self)>(self).value() );
  }

  ALWAYS_INLINE auto operator*(this auto&& self) -> decltype(auto) {
    self._throw_if_inactive();
    return std::forward<decltype(self)>(self).value();
  }

  template<class O>
  ALWAYS_INLINE auto operator==(const O& other) -> bool {
    return has_value_ && value() == other;
  }

  template<class O>
  ALWAYS_INLINE auto operator==(const Optional<O>& other) -> bool {
    return has_value_ == other.has_value_ && (!has_value_ || value() == other.value());
  }

  ALWAYS_INLINE auto operator=(Optional&& other) -> Optional& {
    clear();
    if(other.has_value_) {
      has_value_ = true;
      std::construct_at<T>( reinterpret_cast<T*>(&value_), other.release_value() );
    }
    return *this;
  }

  ALWAYS_INLINE auto operator=(const Optional& other) -> Optional& {
    clear();
    if(other.has_value_) {
      has_value_ = true;
      std::construct_at<T>( reinterpret_cast<T*>(&value_), other.value() );
    }
    return *this;
  }

  template<class F>
  ALWAYS_INLINE auto and_then(this auto&& self, F&& fn) -> decltype(self) {
    if(self.has_value_)
      fn( std::forward<decltype(self)>(self).value() );
    return self;
  }

  template<class F>
  ALWAYS_INLINE auto transform(this auto&& self, F&& fn) -> decltype(auto) {
    using SelfType   = decltype(self);
    using ReturnType = decltype( fn(self.value()) );
    using OptionType = Optional<ReturnType>;
    if(self.has_value_)
      return OptionType{ fn(std::forward<SelfType>(self).value()) };
    else
      return OptionType{};
  }

  template<class ...Args>
  ALWAYS_INLINE auto emplace(Args&&... args) -> void {
    clear();           // Clear the existing value.
    has_value_ = true; // maintain the value state.
    std::construct_at<T>(reinterpret_cast<T*>(&value_), std::forward<Args>(args)...);
  }

  template<class ...Args>
  ALWAYS_INLINE Optional(Args&&... args) {
    std::construct_at<T>(reinterpret_cast<T*>(&value_), std::forward<Args>(args)...);
    has_value_ = true;
  }

  ALWAYS_INLINE Optional(const Optional& other) {
    if(!other.has_value_) return;
    std::construct_at<T>( reinterpret_cast<T*>(&value_), other.value() );
    has_value_ = true;
  }

  ALWAYS_INLINE Optional(Optional&& other) {
    if(!other.has_value_) return;
    std::construct_at<T>( reinterpret_cast<T*>(&value_), other.release_value() );
    has_value_ = true;
  }

  ALWAYS_INLINE auto release_value() -> ValueType {
    _throw_if_inactive();
    T released = std::move(value());
    value().~T();
    has_value_ = false;
    return released;
  }

  auto _throw_if_inactive() const -> void {
    if(!has_value_)
      throw std::runtime_error("Bad Optional access");
  }

  ALWAYS_INLINE auto clear() -> void {
    if(has_value_)
      value().~T();
    has_value_ = false;
  }

  auto has_value() const -> bool { return has_value_; }
  explicit operator bool() const { return has_value_; }

  Optional()                 : has_value_(false) {}
  Optional(const __Nullopt&) : has_value_(false) {}
 ~Optional() { clear(); }
private:
  bool has_value_;
  alignas(T) uint8_t value_[ sizeof(T) ]{};
};

#endif //OPTIONAL_HPP
