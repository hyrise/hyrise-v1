#pragma once

#include <thread>
#include <atomic>
#include <immintrin.h>
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
