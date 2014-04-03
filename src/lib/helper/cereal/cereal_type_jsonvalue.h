#ifndef CEREAL_TYPES_JSONVALUE_H_
#define CEREAL_TYPES_JSONVALUE_H_

#include <jsoncpp/json.h>


static const int JSON_VALUE_TYPE_ARRAY = 4;

namespace cereal {
//! Loading Json::Value
template <class Archive>
inline void load(Archive& ar, Json::Value& val) {

  int valueType = ar.valueType();

  if (valueType == JSON_VALUE_TYPE_ARRAY) {
    Json::Reader reader;
    size_type size;
    std::vector<Json::Value> values;
    std::string str_val;

    ar(make_size_tag(size));

    values.resize(static_cast<std::size_t>(size));
    for (auto it = values.begin(), end = values.end(); it != end; ++it) {
      str_val = ar.parseObjectToString();
      reader.parse(str_val, *it);
    }

    for (auto jason_value : values)
      val.append(jason_value);
  } else {
    std::cerr << "Error: Unable to serialize given type into Json::Value." << std::endl;
  }
}
}  // namespace cereal

#endif  // CEREAL_TYPES_JSONVALUE_H_
