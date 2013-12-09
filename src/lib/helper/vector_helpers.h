#pragma once

#include <algorithm>
#include <functional>
#include <numeric>
#include <iterator>

namespace hyrise { namespace functional {

	template <typename T, typename F>
	auto collect(const T& values, F func) -> std::vector<decltype(func(*std::begin(values)))> {
		typedef std::vector<decltype(func(*std::begin(values)))> result_type;
		result_type result(values.size());
		std::transform(std::begin(values), std::end(values), std::begin(result), func);
		return result;
	}

	template<typename T, typename U, typename F>
	auto foldLeft(const T& values, U initial, F func ) -> U {
		return std::accumulate(values.begin(), values.end(), initial, func);
	}

	template<typename T, typename F>
	auto select(const T& values, F func) -> T {
		T result;
		std::copy_if(std::begin(values), std::end(values), std::back_inserter(result), func);
		return result;
	}

	template<typename T, typename U, typename F>
	auto sum(T values, U initial, F func ) -> U {
		using VT = decltype(*std::begin(values));
    return std::accumulate(values.begin(), values.end(), initial, [func](U l, VT v) ->U {
      return l + func( v);
	  });
  }

  template<typename T, typename F>
  void forEachWithIndex(T values, F func) {
  	auto begin = std::begin(values);
  	auto end = std::end(values);
  	for(size_t index = 0; begin != end; ++index, ++begin) {
  		func(index, *begin);
  	}
  }
}}





template <typename T, typename U>
std::vector<std::shared_ptr<T>> convert(const std::vector<std::shared_ptr<U>>& source) {
  return hyrise::functional::collect(source, [] (decltype(*std::begin(source)) item) {
                   return std::dynamic_pointer_cast<T>(item);
                 });
}


template <typename Input>
bool allValid(Input values) {
  return std::all_of(std::begin(values), std::end(values), [] (decltype(*begin(values)) v) { return v != nullptr; });
}

template <typename MapType>
auto getOrDefault(const MapType& map, typename MapType::key_type key, typename MapType::mapped_type def) -> typename MapType::mapped_type {
  auto f = map.find(key);
  if (f != std::end(map)) {
    return f->second;
  }
  return def;
}

