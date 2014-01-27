#pragma once

#include "SparseVector.h"

template <typename T>
inline bool atomic_cas(T* ptr, T oldV, T newV) {
	return __sync_bool_compare_and_swap(ptr, oldV, newV);
}


template<typename T, typename V>
inline bool atomic_cas_vector(T& vector, size_t index, V oldV, V newV) {
  return atomic_cas(&vector[index], oldV, newV);
}


template<>
inline bool atomic_cas_vector<hyrise::helper::SparseVector<hyrise::tx::transaction_id_t>, hyrise::tx::transaction_id_t>(hyrise::helper::SparseVector<hyrise::tx::transaction_id_t>& vector, size_t index, hyrise::tx::transaction_id_t oldVal, hyrise::tx::transaction_id_t newVal) {
  return vector.cmpxchg(index, oldVal, newVal);
}
