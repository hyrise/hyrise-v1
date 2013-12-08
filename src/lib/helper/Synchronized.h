#pragma once

#include <memory>

template<typename T, typename LockT>
struct Synchronized {
 private:
  mutable T _t;
  mutable LockT _mtx;
 public:
  Synchronized() {}
  explicit Synchronized(T t) : _t(t) {}

  template<typename Functor>
  inline auto operator()( Functor&& f ) const -> decltype(f(_t)) {
    std::lock_guard<LockT> guard(_mtx);
    return f(_t);
  }

  Synchronized& operator=(Synchronized const&) = delete;
  Synchronized& operator=(Synchronized &&) = delete;
  Synchronized(Synchronized const&) = delete;
  Synchronized(Synchronized &&) = delete;
};

