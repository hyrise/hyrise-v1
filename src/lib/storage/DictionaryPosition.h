
#ifndef SRC_LIB_STORAGE_DICTIONARYPOSITION_H_
#define SRC_LIB_STORAGE_DICTIONARYPOSITION_H_

template <typename T>
class DictionaryPosition {
 public:
  size_t index;
  DictionaryIterator<T> it;
  DictionaryPosition(size_t _index, DictionaryIterator<T> _it) : index(_index), it(_it) {}

  bool operator<(const DictionaryPosition &other) const {
    return *it > *(other.it);
  }

  inline T getValue() {
    return *it;
  }
};

#endif  // SRC_LIB_STORAGE_DICTIONARYPOSITION_H_
