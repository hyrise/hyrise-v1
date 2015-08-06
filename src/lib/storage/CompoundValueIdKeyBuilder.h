// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include "storage/storage_types.h"
#include "storage/AbstractTable.h"

namespace hyrise {
namespace storage {

class CompoundValueIdKeyBuilder {
  // This builds a compound_valueid_key_t with all included value ids.
  // By passing the size of the dictionary, it can calculate the number of needed bits
  // in order to avoid storing unneeded zero bits. Of course, this only works
  // with fixed-size dictionaries.

 private:
  compound_valueid_key_t _key;
  size_t _offset;

  int bits_for_dict(size_t size) {
    // calculates the number of bits needed to represent a dictionary with a given size
    if (size < 2)
      return 0;  // if a column only has one value, we can ignore it for the index

    int r = 0;

    while (size >>= 1)
      r++;  // we want the logarithm of the last value id, not the size

    return r + 1;
  }

 public:
  CompoundValueIdKeyBuilder() : _key(0), _offset(0) {}
  compound_valueid_key_t getValidBitMaskForCurrentKey() {
    // generates a key with all bits set which belong to the current key.
    compound_valueid_key_t mask = (1UL << (64 - _offset)) - 1UL;
    // todo: does this work for x>32?
    return ~mask;
  }

  void add(value_id_t value_id, size_t dict_size) {
    int remaining_size = sizeof(compound_valueid_key_t) * 8 - _offset;
    int bits = bits_for_dict(dict_size);

    if (bits > remaining_size) {
      throw std::runtime_error("Maximum size of compound key exceeded");
    }

    size_t no_of_shifts = remaining_size - bits;
    compound_valueid_key_t value_id_shifted = (compound_valueid_key_t)value_id << no_of_shifts;
    _key |= value_id_shifted;
    _offset += bits;
  }

  compound_valueid_key_t get() { return _key; }
};
}
}
