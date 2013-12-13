// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include <storage/ColumnMetadata.h>

#include <string>
#include <sstream>
#include <stdexcept>

#include <boost/algorithm/string.hpp>

namespace hyrise {
namespace storage {

ColumnMetadata *ColumnMetadata::metadataFromString(std::string typestring, std::string name) {
  boost::trim(typestring);
  boost::trim(name);

  if (typestring.compare(STR_TYPE_INTEGER) == 0) {
    return new ColumnMetadata(name, IntegerType);
  } else if (typestring.compare(STR_TYPE_FLOAT) == 0) {
    return new ColumnMetadata(name, FloatType);
  } else if (typestring.compare(STR_TYPE_STRING) == 0) {
    return new ColumnMetadata(name, StringType);
  } else {
    std::stringstream msg;
    msg << "Typename not supported: " << typestring << " with name: " << name << std::endl;
    throw ColumnMetaCreationException(msg.str());
  }

}

} } // namespace hyrise::storage

