#ifndef SRC_LIB_STORAGE_TABLEGENERATOR_IMPL_H_
#define SRC_LIB_STORAGE_TABLEGENERATOR_IMPL_H_

#include <sstream>

#include "AbstractDictionary.h"
#include "ColumnMetadata.h"
#include "OrderIndifferentDictionary.h"
#include "Table.h"



template<typename Strategy>
AbstractTable::SharedTablePtr TableGenerator::create_empty_base_table_modifiable(size_t rows, size_t cols) {
  typedef Table<Strategy> strategy_table;
  auto dicts = new std::vector<AbstractTable::SharedDictionaryPtr>;
  auto md = new metadata_list;

  for (size_t col = 0; col < cols; ++col) {
    std::stringstream s;
    s << "attr" << col;
    md->push_back(new ColumnMetadata(s.str(), IntegerType));
    dicts->push_back(AbstractDictionary::dictionaryWithType<DictionaryFactory<OrderIndifferentDictionary> >(IntegerType));
  }

  auto new_table = std::make_shared<strategy_table>(md, dicts, rows, false);

  delete dicts;
  delete md;
  new_table->resize(rows);
  return new_table;
}

#endif  // SRC_LIB_STORAGE_TABLEGENERATOR_IMPL_H_
