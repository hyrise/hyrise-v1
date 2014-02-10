// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <storage/AbstractTable.h>

#include <storage/MutableVerticalTable.h>
#include <storage/Table.h>
#include <storage/DictionaryFactory.h>
#include <storage/Store.h>
#include <storage/ColumnMetadata.h>
#include <storage/ColumnMetadata.h>
#include <storage/storage_types.h>
#include <storage/PrettyPrinter.h>
#include <storage/storage_types_helper.h>
#include <storage/TableDiff.h>
#include <storage/TableUtils.h>
#include <storage/meta_storage.h>

#include <helper/locking.h>

#include <fstream>

#include <iostream>

namespace hyrise {
namespace storage {

atable_ptr_t AbstractTable::copy_structure(const field_list_t *fields, const bool reuse_dict, const size_t initial_size, const bool with_containers, const bool compressed) const {
  std::vector<ColumnMetadata > metadata;
  std::vector<AbstractTable::SharedDictionaryPtr> *dictionaries = nullptr;

  if (reuse_dict) {
    dictionaries = new std::vector<AbstractTable::SharedDictionaryPtr>();
  }

  if (fields != nullptr) {
for (const field_t & field: *fields) {
      metadata.push_back(metadataAt(field));

      if (dictionaries != nullptr) {
        dictionaries->push_back(dictionaryAt(field));
      }
    }
  } else {
    for (size_t i = 0; i < columnCount(); ++i) {
      metadata.push_back(metadataAt(i));

      if (dictionaries != nullptr) {
        dictionaries->push_back(dictionaryAt(i));
      }
    }
  }

  auto res =  std::make_shared<Table>(&metadata, dictionaries, initial_size, true, compressed);
  delete dictionaries;
  return res;
}

atable_ptr_t AbstractTable::copy_structure_modifiable(const field_list_t *fields, const size_t initial_size, const bool with_containers) const {
  std::vector<ColumnMetadata > metadata;
  auto dictionaries = std::unique_ptr<std::vector<AbstractTable::SharedDictionaryPtr > >(new std::vector<AbstractTable::SharedDictionaryPtr >);

  if (fields != nullptr) {
    for (const field_t & field: *fields) {
      auto m = metadataAt(field);
      m.setType(types::getUnorderedType(m.getType()));
      metadata.push_back(m);
      dictionaries->push_back(makeDictionary(m));
    }
  } else {
    for (size_t i = 0; i < columnCount(); ++i) {
      auto m = metadataAt(i);
      m.setType(types::getUnorderedType(m.getType()));
      metadata.push_back(m);
      dictionaries->push_back(makeDictionary(m));
    }
  }


  auto result = std::make_shared<Table>(&metadata, dictionaries.get(), initial_size, false);
  return result;
}

std::vector<AbstractTable::SharedDictionaryPtr> *AbstractTable::dictionaries() const {
  auto result = new std::vector<AbstractTable::SharedDictionaryPtr>();

  for (size_t c = 0; c < columnCount(); c++) {
    result->push_back(dictionaryAt(c));
  }

  return result;
}

ValueIdList AbstractTable::copyValueIds(const size_t row, const field_list_t *fields) const {
  ValueIdList valueIdList;

  if (fields) {
for (const field_t & field: *fields) {
      valueIdList.push_back(getValueId(field, row));
    }
  } else {
    for (size_t i = 0; i < columnCount(); ++i) {
      valueIdList.push_back(getValueId(i, row));
    }
  }

  return valueIdList;
}

std::string AbstractTable::printValue(const size_t column, const size_t row) const {
  return HyriseHelper::castValueByColumnRow<std::string>(this, column, row);
}

void AbstractTable::copyValueFrom(const c_atable_ptr_t& source, const size_t src_col, const size_t src_row, const size_t dst_col, const size_t dst_row) {

  switch (source->typeOfColumn(src_col)) {
  case IntegerType:
  case IntegerTypeDelta:
  case IntegerTypeDeltaConcurrent:
    copyValueFrom<hyrise_int_t>(source, src_col, src_row, dst_col, dst_row);
    break;
  case IntegerNoDictType:
    copyValueFrom<hyrise_int32_t>(source, src_col, src_row, dst_col, dst_row);
    break;
  case FloatType:
  case FloatTypeDelta:
  case FloatTypeDeltaConcurrent:
  case FloatNoDictType:
    copyValueFrom<hyrise_float_t>(source, src_col, src_row, dst_col, dst_row);
    break;
  case StringType:
  case StringTypeDelta:
  case StringTypeDeltaConcurrent:
    copyValueFrom<hyrise_string_t>(source, src_col, src_row, dst_col, dst_row);
    break;
  }
}

void AbstractTable::copyRowFrom(const c_atable_ptr_t& source, const size_t src_row, const size_t dst_row, const bool copy_values, const bool use_memcpy) {
  if (copy_values) {
    for (size_t column = 0; column < source->columnCount(); column++) {
      copyValueFrom(source, column, src_row, column, dst_row);
    }
  } else {
    // Copy single values
    for (size_t column = 0; column < source->columnCount(); column++) {
     setValueId(column, dst_row, source->getValueId(column, src_row));
   }
 }
}

void AbstractTable::setValueId(const size_t column, const size_t row, const ValueId valueId) {
  throw std::runtime_error("Setting valueIds not supported");
}

void AbstractTable::reserve(const size_t nr_of_values) {
  throw std::runtime_error("Reserving is not supported for Abstract Tables");
}

void AbstractTable::resize(const size_t) {
  throw std::runtime_error("Resizing is not supported for Abstract Tables");
}


void AbstractTable::write(const std::string &filename) const {
  std::ofstream file(filename.c_str());

  for (size_t column = 0; column < columnCount(); column++) {
    auto md = metadataAt(column);
    file << md.getName();

    if (column < columnCount() - 1) {
      file << "|";
    }
  }

  file << std::endl;

  for (size_t column = 0; column < columnCount(); column++) {
    auto md = metadataAt(column);
    file << data_type_to_string(md.getType());

    if (column < columnCount() - 1) {
      file << "|";
    }
  }

  file << std::endl;

  for (size_t column = 0; column < columnCount(); column++) {
    file << column << "_R";

    if (column < columnCount() - 1) {
      file << "|";
    }
  }

  file << std::endl;

  file << "===" << std::endl;

  for (size_t row = 0; row < size(); row++) {
    for (size_t column = 0; column < columnCount(); column++) {
      file << printValue(column, row);

      if (column < columnCount() - 1) {
        file << "|";
      }
    }

    file << std::endl;
  }

  file.close();
}


bool AbstractTable::contentEquals(const c_atable_ptr_t& other) const {

  if (size() != other->size()) {
    return false;
  }

  if (columnCount() != other->columnCount()) {
    return false;
  }

  for (size_t column = 0; column < columnCount(); column++) {
    auto md = metadataAt(column);
    auto md2 = other->metadataAt(column);

    if (!types::isCompatible(md.getType(),md2.getType())) {
      return false;
    }

    if (md.getName() != md2.getName()) {
      return false;
    }

    for (size_t row = 0; row < size(); row++) {
      bool valueEqual = false;

      switch (md.getType()) {
      case IntegerType:
      case IntegerTypeDelta:
      case IntegerTypeDeltaConcurrent:
      case IntegerNoDictType:
	valueEqual = getValue<hyrise_int_t>(column, row) == other->getValue<hyrise_int_t>(column,
                       row);
          break;

      case FloatType:
      case FloatTypeDelta:
      case FloatTypeDeltaConcurrent:
          valueEqual = getValue<hyrise_float_t>(column, row) == other->getValue<hyrise_float_t>(column, row);
          break;

      case StringType:
      case StringTypeDelta:
      case StringTypeDeltaConcurrent:
          valueEqual = getValue<std::string>(column, row).compare(other->getValue<std::string>(column, row)) == 0;
          break;

        default:
          break;
      }

      if (!valueEqual) {
        return false;
      }
    }
  }

  return true;
}

size_t AbstractTable::numberOfColumn(const std::string &column) const {
  for (size_t i = 0; i != columnCount(); i++) {
    if (metadataAt(i).getName() == column) {
      return i;
    }
  }

  std::string err = "";

  for (size_t i = 0; i != columnCount(); i++) {
    err += " " + metadataAt(i).getName();
  }

  throw MissingColumnException("Column " + column + " not found. Available: " + err);
}

void AbstractTable::print(const size_t limit) const {
  PrettyPrinter::print(this, std::cout, "unnamed abstract table", limit);
}

DataType AbstractTable::typeOfColumn(const size_t column) const {
  return metadataAt(column).getType();
}

std::string AbstractTable::nameOfColumn(const size_t column) const {
  return metadataAt(column).getName();
}

metadata_vec_t AbstractTable::metadata() const {
  metadata_vec_t result(columnCount());
  for(size_t i=0; i < result.size(); ++i)
    result[i] = metadataAt(i);
  return result;
}

const attr_vectors_t AbstractTable::getAttributeVectors(size_t column) const {
  throw std::runtime_error("getAttributeVectors not implemented");
}

void AbstractTable::debugStructure(size_t level) const {
  std::cout << std::string(level, '\t') << "AbstractTable " << this << std::endl;
}

unique_id AbstractTable::getUuid() const {
  return _uuid;
}

void AbstractTable::setUuid(unique_id u) {
  if (u.is_nil()) {
    _uuid = unique_id::create();
  }
  _uuid = u;
}

} } // namespace hyrise::storage

