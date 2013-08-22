#ifndef SRC_LIB_HELPER_CAS_H_
#define SRC_LIB_HELPER_CAS_H_

template <typename T>
inline bool atomic_cas(T* ptr, T oldV, T newV) {
	return __sync_bool_compare_and_swap(ptr, oldV, newV);
}

#endif