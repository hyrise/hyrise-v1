#ifndef SRC_LIB_HELPER_RANGEITER_H_
#define SRC_LIB_HELPER_RANGEITER_H_

/// Range iterator to be used for numerical ranges
///
/// Useful for interactions with the STL
/// std::accumulate(RangeIter(0), RangeIter(10)) will
/// return the sum of numbers 0 to inclusively 9
class RangeIter {
 private:
  size_t _value;
 public:
  RangeIter() {}
  RangeIter(size_t val) : _value(val) {}

  inline bool operator==(const RangeIter& other) {
    return _value == other._value;
  }

  inline bool operator!=(const RangeIter& other) {
    return _value != other._value;
  }

  inline size_t operator*() {
    return _value;
  }

  inline void operator++() {
    _value++;
  }
};

#endif
