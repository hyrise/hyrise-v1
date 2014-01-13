// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "io/MetadataCreation.h"

#include "helper/stringhelpers.h"

#include "storage/AbstractTable.h"
#include "storage/ColumnMetadata.h"

namespace hyrise {
namespace io {

const std::string RowType = "R";
const std::string ColType = "C";
const std::string TypeSeparator = "_";

void checkTypePartErrors(const std::vector<std::string> &type_parts, const std::string &original) {
  if (type_parts.size() != 2) {
    throw MetadataCreationError("Structure specification does not match {colnr}_{type}, but is '" + original + "'");
  }

  if (type_parts[1] != RowType && type_parts[1] != ColType) {
    throw MetadataCreationError("Structure column specification is not 'R' or 'C', but is '" + original + "'");
  }
}

typedef std::vector<std::string> line_t;

storage::compound_metadata_list *createMetadata(const std::vector<std::vector<std::string> > &lines,
                                                storage::c_atable_ptr_t tab) {
  line_t names(lines[0]);
  line_t types(lines[1]);
  line_t structure(lines[2]);

  if (names.size() != types.size()
      || names.size() != structure.size()) {
    throw MetadataCreationError("Names, types and structure information do not match in length");
  }

  int last_part = 0;
  auto *current = new storage::metadata_list();
  auto *result = new std::vector<storage::metadata_list *>();
  for (size_t i = 0; i < names.size(); ++i) {
    std::vector<std::string> type_parts;
    splitString(type_parts, structure[i], TypeSeparator);
    checkTypePartErrors(type_parts, structure[i]);

    int part = fromString<int>(type_parts[0]);

    if (part != last_part && current->size() > 0) {
      result->push_back(current);
      current = new storage::metadata_list();
      last_part = part;
    }

    storage::ColumnMetadata metadata;
    if (tab == nullptr)
      metadata = storage::ColumnMetadata::metadataFromString(types[i], names[i]);
    else
      metadata = storage::ColumnMetadata(names[i], tab->typeOfColumn(tab->numberOfColumn(names[i])));

    current->push_back(metadata);
  }

  if (current->size() > 0) {
    result->push_back(current);
  }

  return result;
}

} } // namespace hyrise::io

