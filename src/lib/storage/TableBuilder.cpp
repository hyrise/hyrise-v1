// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "storage/TableBuilder.h"

#include "storage/AbstractTable.h"
#include "storage/DictionaryFactory.h"
#include "storage/OrderIndifferentDictionary.h"
#include "storage/Table.h"
#include "storage/MutableVerticalTable.h"

namespace hyrise { namespace storage {

void TableBuilder::checkParams(const param_list &args) {
  if (args.size() == 0)
    throw TableBuilderError("Cannot build a table with no columns");

  // TODO check if the number of columns in groups is correct
  size_t sum = 0;
for (const auto & i: args.groups())
    sum += i;

  if (sum != args.size())
    throw TableBuilderError("Specified Layout does not match number of columns");
}

atable_ptr_t TableBuilder::createTable(param_list::param_list_t::const_iterator begin,
    param_list::param_list_t::const_iterator end,
    const bool compressed) {
  // Meta data container
  std::vector<ColumnMetadata > vc;
  std::vector<AbstractTable::SharedDictionaryPtr > vd;

  for (; begin != end; ++begin) {
    vc.push_back(ColumnMetadata::metadataFromString((*begin).type, (*begin).name));
    vd.push_back(makeDictionary(vc.back().getType()));
  }

  auto tmp = std::make_shared<Table>(&vc, &vd, 0, 0, compressed);

  return tmp;
}

atable_ptr_t TableBuilder::build(param_list args, const bool compressed) {
  if (args.groups().size() == 0)
    args.appendGroup(args.size());

  checkParams(args);

  std::vector<atable_ptr_t> base;
  auto offset = args.params().begin();

  // For each group calculate the offset that is used to extract the columns
  for (size_t g = 0; g < args.groups().size(); ++g) {

    // Calculate the upper bound for the current layout
    auto end = offset;
    auto tmp = args.groups()[g];
    while(tmp-- != 0)
      ++end;

    base.push_back(createTable(offset, end, compressed));
    offset = end;
  }

  return std::move(std::make_shared<MutableVerticalTable>(base));
}


}}

