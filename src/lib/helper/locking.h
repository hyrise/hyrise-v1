#ifndef SRC_LIB_HELPER_LOCKING_H_
#define SRC_LIB_HELPER_LOCKING_H_

#include <atomic>

namespace hyrise { namespace locking {

	class Spinlock {
	private: 
		typedef enum {Locked, Unlocked} LockState;
		std::atomic<LockState> _state;

	public:
		Spinlock() : _state(Unlocked){}

		void lock() {
			while(!tryLock()) {
				std::this_thread::yield();
			}
		}

		bool isLocked() {
			return _state.load() == Locked;
		}

	  bool tryLock() {
	  	return !_state.exchange(Locked, std::memory_order_acquire) == Locked; 
	  }
  
	  void unlock() {
	  	_state.store(Unlocked, std::memory_order_release);
	  }
	};


	template<typename LockType>
	class ScopedLock {

		LockType& lt;

	public:

		explicit ScopedLock(LockType& l):lt(l) {
		}

		ScopedLock(){ lt.lock(); }

		~ScopedLock() { lt.unlock(); }

	};

}}

#endif // SRC_LIB_HELPER_LOCKING_H_