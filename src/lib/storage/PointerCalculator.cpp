// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "storage/PointerCalculator.h"

#include <iostream>
#include <string>
#include <unordered_set>

#include "helper/make_unique.h"
#include "helper/checked_cast.h"
#include "helper/PositionsIntersect.h"

#include "storage/PrettyPrinter.h"
#include "storage/Store.h"
#include "storage/TableRangeView.h"

namespace hyrise {
namespace storage {

template <typename T>
T* copy_vec(T* orig) {
  if (orig == nullptr) return nullptr;
  return new T(begin(*orig), end(*orig));
}

void PointerCalculator::unnest() {
  // prevent nested pos_list/fields: if the input table is a
  // PointerCalculator instance, combine the old and new
  // pos_list/fields lists
  if (auto p = std::dynamic_pointer_cast<const PointerCalculator>(table)) {

    // if our actual table is a PC, we have to unfold the positions
    if (pos_list != nullptr && p->pos_list != nullptr) {
      auto tmp_list = new pos_list_t(pos_list->size());
      std::transform(std::begin(*(pos_list)), std::end(*(pos_list)), std::begin(*tmp_list), [p](const pos_t& i) -> pos_t {
	  return p->pos_list->at(i);
	});
      table = p->table;
      std::swap(pos_list, tmp_list);
      delete tmp_list;
    }

    // WARN: There might be a bug here, since it's not clear who owns
    // the memory for the fields pointer
    if (fields != nullptr && p->fields != nullptr) {
      fields = new field_list_t(fields->size());
      for (size_t i = 0; i < fields->size(); i++) {
        (*fields)[i] = p->fields->at(fields->at(i));
      }
      table = p->table;
    }
  }

  if (auto trv = std::dynamic_pointer_cast<const TableRangeView>(table)){
    const auto start =  trv->getStart();
    if (pos_list != nullptr && start != 0) {
      for (size_t i = 0; i < pos_list->size(); i++) {
        (*pos_list)[i] += start;
      }
    }
    table = trv->getTable();
  }
}

PointerCalculator::PointerCalculator(c_atable_ptr_t t, pos_list_t *pos, field_list_t *f) : table(t), pos_list(pos), fields(f) {
  unnest();
  updateFieldMapping();
}

PointerCalculator::PointerCalculator(const PointerCalculator& other) : table(other.table), pos_list(copy_vec(other.pos_list)), fields(copy_vec(other.fields)) {
  updateFieldMapping();
}

atable_ptr_t PointerCalculator::copy() const {
  return create(table, fields, pos_list);
}

PointerCalculator::PointerCalculator(c_atable_ptr_t t, pos_list_t pos) : table(t), pos_list(new pos_list_t(std::move(pos))) {
  unnest();
  updateFieldMapping();
}

PointerCalculator::~PointerCalculator() {
  delete fields;
  delete pos_list;
}

void PointerCalculator::updateFieldMapping() {
  slice_for_slice.clear();
  offset_in_slice.clear();
  width_for_slice.clear();
  slice_count = 0;

  size_t field = 0, dst_field = 0;
  size_t last_field = 0;
  bool last_field_set = false;

  for (size_t src_slice = 0; src_slice < table->partitionCount(); src_slice++) {
    last_field_set = false;
    for (size_t src_field = 0; src_field < table->partitionWidth(src_slice); src_field++) {
      // Check if we have to increase the fields until we reach
      // a projected attribute
      if (fields && (fields->size() <= dst_field || fields->at(dst_field) != field)) {
        field++;
        continue;
      }

      if (!last_field_set || field > last_field + 1) { // new slice
        slice_count++;
        slice_for_slice.push_back(src_slice);
        offset_in_slice.push_back(src_field);
        width_for_slice.push_back(1);
      } else {
        width_for_slice[slice_count - 1] += 1;
      }

      dst_field++;
      last_field = field;

      if (!last_field_set) {
        last_field_set = true;
      }

      field++;
    }
  }
}

void PointerCalculator::setPositions(const pos_list_t pos) {
  if (pos_list != nullptr)
    delete pos_list;
  pos_list = new std::vector<pos_t>(pos);
}

void PointerCalculator::setFields(const field_list_t f) {
  fields = new std::vector<field_t>(f);
  updateFieldMapping();
}


const ColumnMetadata& PointerCalculator::metadataAt(const size_t column_index, const size_t row_index, const table_id_t table_id) const {
  size_t actual_column;

  if (fields) {

    // Check if we have to access a renamed field
    if (_renamed)
      return (*_renamed)[column_index];

    actual_column = fields->at(column_index);
  } else {
    actual_column = column_index;
  }

  if (_renamed)
    return (*_renamed)[actual_column];
  else
    return table->metadataAt(actual_column);
}

void PointerCalculator::setDictionaryAt(AbstractTable::SharedDictionaryPtr dict, const size_t column, const size_t row, const table_id_t table_id) {
  throw std::runtime_error("Can't set PointerCalculator dictionary");
}

const AbstractTable::SharedDictionaryPtr& PointerCalculator::dictionaryAt(const size_t column, const size_t row, const table_id_t table_id) const {
  size_t actual_column, actual_row;

  if (fields) {
    actual_column = fields->at(column);
  } else {
    actual_column = column;
  }

  if (pos_list && pos_list->size() > 0) {
    actual_row = pos_list->at(row);
  } else {
    actual_row = row;
  }

  return table->dictionaryAt(actual_column, actual_row, table_id);
}

const AbstractTable::SharedDictionaryPtr& PointerCalculator::dictionaryByTableId(const size_t column, const table_id_t table_id) const {
  size_t actual_column;

  if (fields) {
    actual_column = fields->at(column);
  } else {
    actual_column = column;
  }

  return table->dictionaryByTableId(actual_column, table_id);
}

size_t PointerCalculator::size() const {
  if (pos_list) {
    return pos_list->size();
  }

  return table->size();
}

size_t PointerCalculator::columnCount() const {
  if (fields) {
    return fields->size();
  }

  return table->columnCount();
}

ValueId PointerCalculator::getValueId(const size_t column, const size_t row) const {
  size_t actual_column, actual_row;

  if (pos_list) {
    actual_row = pos_list->at(row);
  } else {
    actual_row = row;
  }

  if (fields) {
    actual_column = fields->at(column);
  } else {
    actual_column = column;
  }

  return table->getValueId(actual_column, actual_row);
}

unsigned PointerCalculator::partitionCount() const {
  return slice_count;
}

size_t PointerCalculator::partitionWidth(const size_t slice) const {
  // FIXME this should return the width in bytes for the column
  if (fields) {
    return width_for_slice[slice];
  }

  return table->partitionWidth(slice);
}


void PointerCalculator::print(const size_t limit) const {
  PrettyPrinter::print(this, std::cout, "unnamed pointer calculator", limit);
}

size_t PointerCalculator::getTableRowForRow(const size_t row) const
{
  size_t actual_row;
  // resolve mapping of THIS pointer calculator
  if (pos_list) {
    actual_row = pos_list->at(row);
  } else {
    actual_row = row;
  }
  // if underlying table is PointerCalculator, resolve recursively
  auto p = std::dynamic_pointer_cast<const PointerCalculator>(table);
  if (p)
    actual_row = p->getTableRowForRow(actual_row);

  return actual_row;
}

size_t PointerCalculator::getTableColumnForColumn(const size_t column) const
{
  size_t actual_column;
  // resolve field mapping of THIS pointer calculator
  if (fields) {
    actual_column = fields->at(column);
  } else {
    actual_column = column;
  }
  // if underlying table is PointerCalculator, resolve recursively
  auto p = std::dynamic_pointer_cast<const PointerCalculator>(table);
  if (p)
    actual_column = p->getTableColumnForColumn(actual_column);
  return actual_column;
}

c_atable_ptr_t PointerCalculator::getTable() const {
  return table;
}

c_atable_ptr_t PointerCalculator::getActualTable() const {
  auto p = std::dynamic_pointer_cast<const PointerCalculator>(table);

  if (!p) {
    return table;
  } else {
    return p->getActualTable();
  }
}

const pos_list_t *PointerCalculator::getPositions() const {
  return pos_list;
}

pos_list_t PointerCalculator::getActualTablePositions() const {
  auto p = std::dynamic_pointer_cast<const PointerCalculator>(table);

  if (!p) {
    return *pos_list;
  }

  pos_list_t result(pos_list->size());
  pos_list_t positions = p->getActualTablePositions();

  for (pos_list_t::const_iterator it = pos_list->begin(); it != pos_list->end(); ++it) {
    result.push_back(positions[*it]);
  }

  return result;
}


//FIXME: Template this method
atable_ptr_t PointerCalculator::copy_structure(const field_list_t *fields, const bool reuse_dict, const size_t initial_size, const bool with_containers, const bool compressed) const {
  std::vector<ColumnMetadata > metadata;
  std::vector<AbstractTable::SharedDictionaryPtr> *dictionaries = nullptr;

  if (reuse_dict) {
    dictionaries = new std::vector<AbstractTable::SharedDictionaryPtr>();
  }

  if (fields != nullptr) {
    for (const field_t & field: *fields) {
      metadata.push_back(metadataAt(field));

      if (dictionaries != nullptr) {
        dictionaries->push_back(dictionaryAt(field, 0, 0));
      }
    }
  } else {
    for (size_t i = 0; i < columnCount(); ++i) {
      metadata.push_back(metadataAt(i));

      if (dictionaries != nullptr) {
        dictionaries->push_back(dictionaryAt(i, 0, 0));
      }
    }
  }

  auto result = std::make_shared<Table>(&metadata, dictionaries, initial_size, true, compressed);
  delete dictionaries;
  return result;
}

std::shared_ptr<PointerCalculator> PointerCalculator::intersect(const std::shared_ptr<const PointerCalculator>& other) const {
  pos_list_t *result = new pos_list_t();
  result->reserve(std::max(pos_list->size(), other->pos_list->size()));
  assert(std::is_sorted(begin(*pos_list), end(*pos_list)) && std::is_sorted(begin(*other->pos_list), end(*other->pos_list)) && "Both lists have to be sorted");
  
  intersect_pos_list(
    pos_list->begin(), pos_list->end(),
    other->pos_list->begin(), other->pos_list->end(),
    std::back_inserter(*result));

  assert((other->table == this->table) && "Should point to same table");
  return create(table, result, fields);
}


bool PointerCalculator::isSmaller( std::shared_ptr<const PointerCalculator> lx, std::shared_ptr<const PointerCalculator> rx ) {
  return lx->size() < rx->size() ;
}

std::shared_ptr<const PointerCalculator> PointerCalculator::intersect_many(pc_vector::iterator it, pc_vector::iterator it_end) {
  std::sort(it, it_end, PointerCalculator::isSmaller);
  std::shared_ptr<const PointerCalculator> base = *(it++);
  for (;it != it_end; ++it) {
    base = base->intersect(*it);
  }
  return base;
}

std::shared_ptr<PointerCalculator> PointerCalculator::unite(const std::shared_ptr<const PointerCalculator>& other) const {
  assert((other->table == this->table) && "Should point to same table");
  if (pos_list && other->pos_list) {
    auto result = new pos_list_t();
    result->reserve(pos_list->size() + other->pos_list->size());
    assert(std::is_sorted(begin(*pos_list), end(*pos_list)) && std::is_sorted(begin(*other->pos_list), end(*other->pos_list)) && "Both lists have to be sorted");
    std::set_union(pos_list->begin(), pos_list->end(),
                   other->pos_list->begin(), other->pos_list->end(),
                   std::back_inserter(*result));
    return create(table, result, copy_vec(fields));
  } else {
    pos_list_t* positions = nullptr;
    if (pos_list == nullptr) { positions = other->pos_list; }
    if (other->pos_list == nullptr) { positions = pos_list; }
    return create(table, copy_vec(positions), copy_vec(fields));
  }
}

std::shared_ptr<PointerCalculator> PointerCalculator::concatenate(const std::shared_ptr<const PointerCalculator>& other) const {
  assert((other->table == this->table) && "Should point to same table");
  std::vector<std::shared_ptr<const PointerCalculator>> v {std::static_pointer_cast<const PointerCalculator>(shared_from_this()), other};
  return PointerCalculator::concatenate_many(begin(v), end(v));
}

std::shared_ptr<const PointerCalculator> PointerCalculator::unite_many(pc_vector::const_iterator it, pc_vector::const_iterator it_end){
  std::shared_ptr<const PointerCalculator> base = *(it++);
  for (;it != it_end; ++it) {
    base = base->unite(*it);
  }
  return base;
}

std::shared_ptr<PointerCalculator> PointerCalculator::concatenate_many(pc_vector::const_iterator it, pc_vector::const_iterator it_end) {
  auto sz = std::accumulate(it, it_end, 0, [] (size_t acc, const std::shared_ptr<const PointerCalculator>& pc) { return acc + pc->size(); });
  auto result = new pos_list_t;
  result->reserve(sz);

  c_atable_ptr_t table = nullptr;
  for (;it != it_end; ++it) {
    const auto& pl = (*it)->pos_list;
    if (table == nullptr) {
      table = (*it)->table;
    }

    if (pl == nullptr) {
      auto sz = (*it)->size();
      result->resize(result->size() + sz);
      std::iota(end(*result)-sz, end(*result), 0);
    } else {
      result->insert(end(*result), begin(*pl), end(*pl));
    }
  }

  return create(table, result, nullptr);
}

void PointerCalculator::debugStructure(size_t level) const {
  std::cout << std::string(level, '\t') << "PointerCalculator " << this << std::endl;
  table->debugStructure(level+1);
}


void PointerCalculator::validate(tx::transaction_id_t tid, tx::transaction_id_t cid) {
  const auto& store = checked_pointer_cast<const Store>(table);
  if (pos_list == nullptr) {
    pos_list = new pos_list_t(store->buildValidPositions(cid, tid));
  } else {
    store->validatePositions(*pos_list, cid, tid);
  }
}

void PointerCalculator::remove(const pos_list_t& pl) {
  std::unordered_set<pos_t> tmp(pl.begin(), pl.end());
  const auto& end = tmp.cend();
  auto res = std::remove_if(std::begin(*pos_list), std::end(*pos_list),[&tmp, &end](const pos_t& p){
    return tmp.count(p) != 0u;
  });
  (*pos_list).erase(res, pos_list->end());
}



void PointerCalculator::rename(field_t f, const std::string newName) {

  if (!_renamed) {
    _renamed = make_unique<std::vector<ColumnMetadata>>(metadata());
  }

  (*_renamed)[f] = ColumnMetadata(newName, table->typeOfColumn(f));
}

} } // namespace hyrise::storage
