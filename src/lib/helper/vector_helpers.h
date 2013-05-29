#ifndef SRC_LIB_HELPERS_VECTOR_HELPERS_H

template <typename T, typename F>
auto collect(T values, F func) -> std::vector<decltype(func(*std::begin(values)))> {
  typedef std::vector<decltype(func(*std::begin(values)))> result_type;
  result_type result(values.size());
  std::transform(std::begin(values), std::end(values), std::begin(result), func);
  return result;
}

template <typename T, typename U>
std::vector<std::shared_ptr<T>> convert(std::vector<std::shared_ptr<U>> source) {
  return collect(source, [] (decltype(*std::begin(source)) item) {
                   return std::dynamic_pointer_cast<T>(item);
                 });
}

template <typename Input>
bool allValid(Input values) {
  return std::all_of(std::begin(values), std::end(values), [] (decltype(*begin(values)) v) { return v != nullptr; });
}

#endif
