// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include <storage/ColumnMetadata.h>

#include <string>
#include <sstream>
#include <stdexcept>

#include <boost/algorithm/string.hpp>

namespace hyrise {
namespace storage {

ColumnMetadata ColumnMetadata::metadataFromString(std::string typestring, std::string name) {
  boost::trim(typestring);
  boost::trim(name);

  if (typestring.compare(types::integer_name) == 0) {
    return ColumnMetadata(name, IntegerTypeDelta);

  } else if (typestring.compare(types::float_name) == 0) {
    return ColumnMetadata(name, FloatTypeDelta);

  } else if (typestring.compare(types::string_name) == 0) {
    return ColumnMetadata(name, StringTypeDelta);

  } else if (typestring.compare(types::no_dict_integer_name) == 0) {
    return ColumnMetadata(name, IntegerNoDictType);

  } else if (typestring.compare(types::no_dict_float_name) == 0) {
    return ColumnMetadata(name, FloatNoDictType);

  } else if (typestring.compare(types::integer_name_main) == 0) {
    return ColumnMetadata(name, IntegerType);

  } else if (typestring.compare(types::float_name_main) == 0) {
    return ColumnMetadata(name, FloatType);

  } else if (typestring.compare(types::string_name_main) == 0) {
    return ColumnMetadata(name, StringType);

  } else if (typestring.compare(types::integer_name_conc) == 0) {
    return ColumnMetadata(name, IntegerTypeDeltaConcurrent);

  } else if (typestring.compare(types::float_name_conc) == 0) {
    return ColumnMetadata(name, FloatTypeDeltaConcurrent);

  } else if (typestring.compare(types::string_name_conc) == 0) {
    return ColumnMetadata(name, StringTypeDeltaConcurrent);
  } else {
    std::stringstream msg;
    msg << "Typename not supported: " << typestring << " with name: " << name << std::endl;
    throw ColumnMetaCreationException(msg.str());
  }

}

} } // namespace hyrise::storage

