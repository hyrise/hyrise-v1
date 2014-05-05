#ifndef CEREAL_TYPES_OPTIONAL_H_
#define CEREAL_TYPES_OPTIONAL_H_

#include <optional/optional.hpp>

namespace cereal {
//! Loading std::optional
template <class Archive, class T>
inline void load(Archive& ar, std::optional<T>& opt) {
  ar.setOptional(true);
  T optValue;
  ar(optValue);
  if (ar.foundOptionalValue())
    opt = std::optional<T>(optValue);
  ar.setOptional(false);
}
}  // namespace cereal

#endif  // CEREAL_TYPES_OPTIONAL_H_
