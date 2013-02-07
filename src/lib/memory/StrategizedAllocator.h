#ifndef SRC_LIB_MEMORY_STRATEGIZEDALLOCATOR_H_
#define SRC_LIB_MEMORY_STRATEGIZEDALLOCATOR_H_

#include <stddef.h>

#include <algorithm>
#include <list>
#include <stdexcept>

template <typename T, typename S>
class StrategizedAllocator {
 public:

  // The following will be the same for virtually all allocators.
  typedef T *pointer;
  typedef const T *const_pointer;
  typedef T &reference;
  typedef const T &const_reference;
  typedef T value_type;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef S strategy;
  typedef S Strategy;

  T *address(T &r) const {
    return &r;
  }

  const T *address(const T &s) const {
    return &s;
  }

  size_t max_size() const {
    return (static_cast<size_t>(0) - static_cast<size_t>(1)) / sizeof(T);
  }

  template <typename U> struct rebind {
    typedef StrategizedAllocator<U, S> other;
  };

  bool operator!=(const StrategizedAllocator &other) const {
    return !(*this == other);
  }

  void construct(T *const p, const T &t) const {
    void *const pv = static_cast<void *>(p);
    new(pv) T(t);
  }

  void construct(T *p) const {
    new((void *) p) T;
  }

  void destroy(T *const p) const;

  // Returns true iff storage allocated from *this
  // can be deallocated from other, and vice versa.
  // Always returns true for stateless allocators.
  bool operator==(const StrategizedAllocator &other) const {
    return true;
  }


  // Default constructor, copy constructor, rebinding constructor, and destructor.
  // Empty for stateless allocators.
  StrategizedAllocator() { }

  StrategizedAllocator(const StrategizedAllocator &) { }

  template <typename U> StrategizedAllocator(const StrategizedAllocator<U, S> &) { }

  ~StrategizedAllocator() { }


  T *allocate(const size_t n) const {
    if (n == 0) {
      return nullptr;
    }

    if (n > max_size()) {
      throw std::length_error("StrategizedAllocator<T>::allocate() - Integer overflow.");
    }

    void *pv = strategy::allocate(n * sizeof(T));

    if (pv == nullptr) {
      throw std::bad_alloc();
    }

    return static_cast<T *>(pv);
  };

  void deallocate(T *const p, const size_t n) const {
    strategy::deallocate(static_cast<void *>(p), n * sizeof(T));
  };

  // The following will be the same for all allocators that ignore hints.
  template <typename U> T *allocate(const size_t n, const U * /* const hint */) const {
    return allocate(n);
  }
 private:
  StrategizedAllocator &operator=(const StrategizedAllocator &);
};


template <typename T, typename S>
void StrategizedAllocator < T, S >::destroy(T *const p) const {
  p->~T();
}

template<typename T1, typename T2>
void assign(T1 &t1, T2 &t2) {
  std::copy(t2.begin(), t2.end(), t1.begin());
}

template<typename T>
void assign(T &t1, T &t2) {
  t1 = t2;
}

#endif  // SRC_LIB_MEMORY_STRATEGIZEDALLOCATOR_H_
