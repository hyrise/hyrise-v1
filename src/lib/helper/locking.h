#pragma once

#include <thread>
#include <atomic>

namespace hyrise { namespace locking {

class Spinlock {
 private:
  typedef enum {Locked, Unlocked} LockState;
  std::atomic<LockState> _state;

 public:
  Spinlock() : _state(Unlocked){}

  void lock() {
    while(!try_lock()) {
      std::this_thread::yield();
    }
  }

  bool is_locked() {
    return _state.load() == Locked;
  }

  bool try_lock() {
    // exchange returns the value before locking, thus we need
    // to make sure the lock wasn't already in Locked state before
    return _state.exchange(Locked, std::memory_order_acquire) != Locked;
  }

  void unlock() {
    _state.store(Unlocked, std::memory_order_release);
  }
};

}}

