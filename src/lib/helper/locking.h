#pragma once

#include <pthread.h>
#include <immintrin.h>

#include <thread>
#include <atomic>

namespace hyrise {
namespace locking {

class Spinlock {
 private:
  typedef enum {
    Locked,
    Unlocked
  } LockState;
  std::atomic<LockState> _state;
  /*the exchange method on this atomic is compiled to a lockfree xchgl instruction*/
 public:
  Spinlock() : _state(Unlocked) {}

  inline void lock() {
    while (!try_lock()) {
      _mm_pause();  // helps the cpu to detect busy-wait loop
    }
  }

  bool is_locked() { return _state.load() == Locked; }

  inline bool try_lock() {
    // exchange returns the value before locking, thus we need
    // to make sure the lock wasn't already in Locked state before
    return _state.exchange(Locked, std::memory_order_acquire) != Locked;
  }

  inline void unlock() { _state.store(Unlocked, std::memory_order_release); }
};
}
}

// Simple RAII-wrapper for pthread_rwlock_t
struct RWMutex {
  mutable pthread_rwlock_t _rw_lock;
  // can only be moved, not copied
 public:
  RWMutex(RWMutex const&) = delete;
  RWMutex& operator=(RWMutex const&) = delete;
  RWMutex() { pthread_rwlock_init(&_rw_lock, nullptr); }
  ~RWMutex() { pthread_rwlock_destroy(&_rw_lock); }
  void lock_shared() const { pthread_rwlock_rdlock(&_rw_lock); }
  void lock_exclusive() const { pthread_rwlock_wrlock(&_rw_lock); }
  void unlock() const { pthread_rwlock_unlock(&_rw_lock); }
};

struct SharedLock {
  const RWMutex& _mtx;
  SharedLock(const RWMutex& mtx) : _mtx(mtx) { mtx.lock_shared(); }
  ~SharedLock() { _mtx.unlock(); }
};

struct ExclusiveLock {
  const RWMutex& _mtx;
  ExclusiveLock(const RWMutex& mtx) : _mtx(mtx) { mtx.lock_exclusive(); }
  ~ExclusiveLock() { _mtx.unlock(); }
};
