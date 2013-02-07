#ifndef SRC_LIB_STORAGE_BITVECTOR_H_
#define SRC_LIB_STORAGE_BITVECTOR_H_

/*

  USAGE:

  BitVector vector;

  vector.reserve(10);

  vector.set(4, true);

  std::cout << vector.get(4) << std::endl;
  std::cout << vector.get(5) << std::endl;

  return 0;

  // prints:

  1
  0

*/

#ifndef BITS_PER_BYTE
#define BITS_PER_BYTE 8
#endif

class BitVector {
 public:
  unsigned char *_values;
  size_t _allocated_bytes;

 public:
  BitVector();
  bool get(size_t index);
  void set(size_t index, bool value);
  void reserve(size_t bytes);
};

BitVector::BitVector() {
  _values = nullptr;
  _allocated_bytes = 0;
}

bool BitVector::get(size_t index) {
  size_t byte_offset = index / BITS_PER_BYTE;
  size_t bit_offset  = index % BITS_PER_BYTE;

  return _values[byte_offset] & (1 << (7 - bit_offset));
}

void BitVector::set(size_t index, bool value) {
  size_t byte_offset = index / BITS_PER_BYTE;
  size_t bit_offset  = index % BITS_PER_BYTE;

  if (value) {
    _values[byte_offset] |= 1 << (7 - bit_offset);
  } else {
    _values[byte_offset] &= ~(1 << (7 - bit_offset));
  }
}

void BitVector::reserve(size_t bytes) {
  void *new_values = realloc(_values, bytes);

  if (new_values == nullptr) {
    free(_values);
    throw std::bad_alloc();
  }

  if (bytes > _allocated_bytes) {
    memset(new_values + _allocated_bytes, 0, bytes - _allocated_bytes);
  }

  _values = (unsigned char *) new_values;
  _allocated_bytes = bytes;
}

#endif  // SRC_LIB_STORAGE_BITVECTOR_H_
