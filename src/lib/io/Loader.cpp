// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "io/Loader.h"

#include <log4cxx/logger.h>

#include "helper/types.h"
#include "io/EmptyLoader.h"
#include "io/LoaderException.h"
#include "storage/AbstractTable.h"
#include "storage/AbstractMergeStrategy.h"
#include "storage/DictionaryFactory.h"
#include "storage/FixedLengthVector.h"
#include "storage/SequentialHeapMerger.h"
#include "storage/SimpleStore.h"
#include "storage/Store.h"
#include "storage/MutableVerticalTable.h"

namespace hyrise {
namespace io {

log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("hyrise.io.Loader"));

param_ref_member_impl(Loader::params, AbstractInput, Input)
param_ref_member_impl(Loader::params, AbstractHeader, Header)
param_member_impl(Loader::params, std::string, BasePath)
param_member_impl(Loader::params, std::string, TableName)
param_member_impl(Loader::params, bool, ModifiableMutableVerticalTable)
param_member_impl(Loader::params, bool, ReturnsMutableVerticalTable)
param_member_impl(Loader::params, bool, Compressed)
param_member_impl(Loader::params, storage::c_atable_ptr_t, ReferenceTable)
param_member_impl(Loader::params, bool, DeltaDataStructure)

Loader::params::params()
    : Input(nullptr),
      Header(nullptr),
      BasePath(""),
      TableName(""),
      ModifiableMutableVerticalTable(false),
      ReturnsMutableVerticalTable(false),
      Compressed(false),
      ReferenceTable(),
      DeltaDataStructure(false) {}

Loader::params::params(const Loader::params& other)
    : BasePath(other.getBasePath()),
      TableName(other.getTableName()),
      ModifiableMutableVerticalTable(other.getModifiableMutableVerticalTable()),
      ReturnsMutableVerticalTable(other.getReturnsMutableVerticalTable()),
      Compressed(other.getCompressed()) {
  if (other.Input != nullptr)
    Input = other.Input->clone();
  if (other.Header != nullptr)
    Header = other.Header->clone();
  if (other.ReferenceTable != nullptr)
    ReferenceTable = other.ReferenceTable;
}

Loader::params& Loader::params::operator=(const Loader::params& other) {
  if (this != &other) {  // protect against invalid self-assignment
    if (Input != nullptr)
      delete Input;
    if (Header != nullptr)
      delete Header;
    Input = other.getInput()->clone();
    Header = other.getHeader()->clone();
    setBasePath(other.getBasePath());
    setTableName(other.getTableName());
    setReturnsMutableVerticalTable(other.getReturnsMutableVerticalTable());
    setModifiableMutableVerticalTable(other.getModifiableMutableVerticalTable());
    setCompressed(other.getCompressed());
    setReferenceTable(other.getReferenceTable());
  }
  // by convention, always return *this
  return *this;
}

Loader::params* Loader::params::clone() const {
  Loader::params* p = new Loader::params();
  if (Input != nullptr)
    p->setInput(*Input);
  if (Header != nullptr)
    p->setHeader(*Header);
  p->setBasePath(BasePath);
  p->setTableName(TableName);
  p->setReturnsMutableVerticalTable(ReturnsMutableVerticalTable);
  p->setModifiableMutableVerticalTable(ModifiableMutableVerticalTable);
  p->setReferenceTable(ReferenceTable);
  p->setCompressed(Compressed);
  return p;
}

Loader::params::~params() {
  if (Input != nullptr)
    delete Input;
  if (Input != nullptr)
    delete Header;
}

std::shared_ptr<storage::AbstractTable> Loader::load(const params& args) {
  AbstractHeader* header = args.getHeader();
  AbstractInput* input = args.getInput();

  // TODO avoid leakage
  if (input == nullptr) {
    input = new EmptyInput();
  }
  if (header == nullptr) {
    header = new EmptyHeader();
  }

  LOG4CXX_DEBUG(logger, "Loading header");
  storage::compound_metadata_list* meta = header->load(args);
  LOG4CXX_DEBUG(logger, "Header done");

  std::vector<storage::atable_ptr_t> tables;
  for (const auto& partition : *meta) {
    std::vector<storage::adict_ptr_t> dicts;
    for (const auto& column : *partition) {
      dicts.push_back(storage::makeDictionary(column));
    }
    auto tbl = std::make_shared<storage::Table>(
        *partition, std::make_shared<storage::FixedLengthVector<value_id_t>>(dicts.size(), 0), dicts);
    tables.push_back(tbl);
  }

  std::shared_ptr<storage::AbstractTable> result,  // initialize empty
      table = std::make_shared<storage::MutableVerticalTable>(tables);

  LOG4CXX_DEBUG(logger, "Loading data");
  try {
    result = input->load(table, meta, args);
  }
  catch (std::exception& e) {
    // TODO: Memory management is not exception safe
    for (const auto& ml : *meta) {
      delete ml;
    }
    throw Loader::Error(e.what());
  }
  // Memory Cleanup, this is so dirty here, it makes me shiver
  for (auto e : *meta) {
    delete e;
  }
  delete meta;

  LOG4CXX_DEBUG(logger, "Data done");

  if (!args.getModifiableMutableVerticalTable() && input->needs_store_wrap()) {
    storage::store_ptr_t s;
    if (args.getTableName().empty()) {
      s = std::make_shared<storage::Store>(result);
    } else {
      s = std::make_shared<storage::Store>(args.getTableName(), result);
    }
    auto merger = new storage::TableMerger(
        new storage::DefaultMergeStrategy(), new storage::SequentialHeapMerger(), args.getCompressed());
    s->setMerger(merger);
    if (input->needs_merge()) {
      s->merge();
    }
    result = s;
  }

  if (!args.getModifiableMutableVerticalTable() && args.getReturnsMutableVerticalTable()) {
    table = std::dynamic_pointer_cast<storage::Store>(result)->getMainTable();
    result = table;
  }

  if (args.getHeader() == nullptr)
    delete header;

  if (args.getInput() == nullptr)
    delete input;

  return result;
}
}
}  // namespace hyrise::io
