// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "MPassCSVLoader.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <algorithm>
#include <set>
#include <thread>

#include "boost/lexical_cast.hpp"
#include "boost/algorithm/string.hpp"

#include "io/GenericCSV.h"
#include "io/ColumnLoader.h"
#include "io/MetadataCreation.h"

#include "storage/AbstractTable.h"
#include "storage/Store.h"
#include "storage/OrderPreservingDictionary.h"
#include "storage/ColumnMetadata.h"
#include "storage/meta_storage.h"

namespace hyrise {
namespace io {

param_member_impl(MPassCSVInput::params, bool, Unsafe);

#define LOAD_SIZE (4096 * 10)

namespace MPassLoader {

struct parallel_data {
  void *vector;
  std::vector<size_t> positions;
  size_t rows;

  // Buffer pointing to the copied data
  char *buffer;

  // Pointer to the current position in the buffer
  char *iter;

  size_t buffSize;

  parallel_data(): vector(nullptr), rows(0), buffer(nullptr), iter(nullptr), buffSize(0)
  {}
};



struct cast_functor {
  typedef void value_type;
  parallel_data *data;

  char tmp[2 * LOAD_SIZE]; // Defines the maximum values size

  explicit cast_functor(parallel_data *d): data(d) {}

  template <typename R>
  inline void operator()() {

    if (data->buffSize == 1 && data->buffer[0] == '\n')
      return;

    typedef std::vector<R> cur_vetor_t;

    if (data->vector == nullptr)
      data->vector = new cur_vetor_t();


    cur_vetor_t *t = (cur_vetor_t *) data->vector;


    size_t last = 0;
    char *citer = tmp;
    for (size_t i = 0; i < data->buffSize; ++i) {
      *citer = data->buffer[i];
      if (data->buffer[i] == '\n') {
        last = i;
        *citer = '\0';
        data->positions.push_back(t->size());
        t->push_back(boost::lexical_cast<R>(tmp));
        ++data->rows;
        citer = tmp;
      } else {
        ++citer;
      }
    }

    // Re-arrange the buffer
    if (last != data->buffSize) {
      size_t offset = data->buffSize - last - 1;
      memcpy(data->buffer, data->buffer + last + 1, offset);
      data->iter = data->buffer + offset;
      data->buffSize = offset;
    } else {
      data->iter = data->buffer;
      data->buffSize = 0;
    }

  }

};

struct do_load_functor {
  typedef void value_type;

  std::shared_ptr<storage::AbstractTable> table;
  parallel_data *data;
  size_t col;

  do_load_functor(std::shared_ptr<storage::AbstractTable> t, parallel_data *d, size_t c):
    table(t), data(d), col(c) {}


  template<typename R>
  void operator()() {
    typedef std::vector<R> cur_vetor_t;
    typedef std::set<R> cur_set_t;
    cur_vetor_t *t = (cur_vetor_t *) data->vector;


    cur_set_t s;
for (auto e : *t)
      s.insert(e);

    auto dict = std::make_shared<storage::OrderPreservingDictionary<R>>();
for (auto e : s)
      dict->addValue(e);

    // Swap dictionary
    table->setDictionaryAt(dict, col, 0);

    for (size_t row = 0; row < t->size(); ++row) {
      table->setValue(col, row, t->at(row));
    }


    // Memory cleanup
    delete t;

  }

};

} // namespace MPassLoader


void parallel_load(std::shared_ptr<storage::AbstractTable> intable, size_t attr, std::string fn, MPassLoader::parallel_data *data) {
  int fp = open(fn.c_str(), O_RDONLY);

  if (fp == -1)
    throw std::runtime_error("Error reading file: " + fn);

  uint64_t size = LOAD_SIZE;
  size_t blocks = 0;


  // Read file in buffer
  size_t result = 0;
  char *buffer = (char *) malloc(size);

  storage::type_switch<hyrise_basic_types> global_ts;


  data->buffer = (char *) malloc(2 * LOAD_SIZE);
  data->iter = data->buffer;

  while ((result = read(fp, buffer, size)) != 0) {

    // Copy the data
    memcpy(data->iter, buffer, result);
    data->buffSize += result;

    MPassLoader::cast_functor fun(data);
    global_ts(intable->typeOfColumn(attr), fun);

    ++blocks;
  }


  // Append last newline
  *data->iter = '\n';
  ++data->buffSize;
  MPassLoader::cast_functor fun(data);
  global_ts(intable->typeOfColumn(attr), fun);

  close(fp);
  free(buffer);
  free(data->buffer);
}

void pass2(std::shared_ptr<storage::AbstractTable> intable, MPassLoader::parallel_data *data, size_t attr) {
  storage::type_switch<hyrise_basic_types> global_ts;
  MPassLoader::do_load_functor loader(intable, data, attr);
  global_ts(intable->typeOfColumn(attr), loader);
}


std::shared_ptr<storage::AbstractTable> MPassCSVInput::load(std::shared_ptr<storage::AbstractTable> intable, const storage::compound_metadata_list *meta, const Loader::params &args) {

  std::vector<std::thread> kThreads(intable->columnCount());
  auto buckets = std::vector<MPassLoader::parallel_data *>(intable->columnCount());

  // Each attribute as a file in the base directory that we read and map
  for (size_t i = 0; i < intable->columnCount(); ++i) {
    std::string fn = _directory + "/" + intable->metadataAt(i).getName() + ".data";
    auto data = new MPassLoader::parallel_data();
    buckets[i] = data;

    kThreads[i] = std::thread(parallel_load, intable, i, fn, data);
  }

  for (size_t i = 0; i < intable->columnCount(); ++i)
    kThreads[i].join();

  intable->reserve(buckets[0]->rows);

  for (size_t i = 0; i < intable->columnCount(); ++i) {
    kThreads[i] = std::thread(pass2, intable, buckets[i], i);
  }

  for (size_t i = 0; i < intable->columnCount(); ++i)
    kThreads[i].join();

  return std::make_shared<storage::Store>(intable);
}

MPassCSVInput *MPassCSVInput::clone() const {
  return new MPassCSVInput(*this);
}

} } // namespace hyrise::io

