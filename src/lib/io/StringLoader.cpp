// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "io/StringLoader.h"

#include <iostream>
#include <sstream>
#include "io/GenericCSV.h"
#include "io/MetadataCreation.h"

compound_metadata_list *StringHeader::load(const Loader::params &args) {
  std::istringstream is(_header);
  std::vector<line_t>lines(csv::parse_stream(is, csv::HYRISE_FORMAT));
  return createMetadata(lines, args.getReferenceTable());
}

StringHeader *StringHeader::clone() const {
  return new StringHeader(*this);
}
