// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <storage/TableGenerator.h>

#include <math.h>
#include <assert.h>
#include <algorithm>
#include <limits>
#include <stdint.h>

#include <random>
#include <sstream>

#include <helper/types.h>
#include <helper/Progress.h>
#include <storage/AbstractTable.h>
#include <storage/DictionaryFactory.h>
#include <storage/MutableVerticalTable.h>
#include <storage/AbstractMergeStrategy.h>
#include <storage/SequentialHeapMerger.h>
#include <storage/TableMerger.h>

namespace hyrise { namespace storage {

TableGenerator::TableGenerator(bool quiet) : _quiet(quiet), _steps(50), _prepareSize(0) {
}

TableGenerator::~TableGenerator() {
}

void TableGenerator::prepare(size_t p) {
  if (p <= _prepareSize)
    return;

  if (_prepareSize > 0) {
    _values.clear();
  }

  _prepareSize = p;

  std::mt19937 gen;
  std::uniform_int_distribution<int64_t> dist(0, std::numeric_limits<int64_t>::max());

  // First, generate all random numbers we need, this is blazing
  // fast
  std::vector<int64_t> v;

  // Depending on the cycle of the random number generator, we will
  // need to generate more random numbers than we actually need to
  // make sure we have still enough without double entries
  int upper = (_prepareSize * 105) / 100;
  for (int i = 0; i < upper; ++i) {
    v.push_back(dist(gen));
  }

  // Now, sort the vector
  std::sort(v.begin(), v.end());
  std::set<int64_t>::iterator it = _values.begin();

  // Insert the elements from the vector sorted to the set and ta da
  // we are fast
  for (size_t i = 0; i < v.size(); ++i) {
    it = _values.insert(it, v[i]);
  }

  // If we have not enough distinct values we will need to generate
  // more random values until we are done, but this should be only a
  // small amount.
  while (_values.size() < _prepareSize) {
    _values.insert(dist(gen));
  }

}

void TableGenerator::start(size_t rows, size_t cols, size_t total) {
  if (!_quiet) {
    std::cout << "Generating Table with " << rows << " rows and " << cols << " columns..." << std::endl;
  }

  _total = total;
  _current = 0;
  _current_step = 0;
}

void TableGenerator::increment() {
  _current++;

  if (_current >= _current_step * _total / 100) {
    if (!_quiet) {
      std::cout << _current_step << "." << std::flush;
    }

    _current_step += 100 / _steps;
  }
}

std::vector<atable_ptr_t > TableGenerator::distinct_cols(size_t cols, size_t main_size, size_t delta_size) {

  atable_ptr_t main = create_empty_table(main_size, cols);
  atable_ptr_t delta = create_empty_table_modifiable(delta_size, cols);

  for (size_t col = 0; col < cols; ++col) {

    if (!_quiet) {
      std::cout << "column " << col << std::endl;
    }

    // generate dict
    size_t distinct = col * 100 / (cols - 1);
    size_t dict_size_main;
    size_t dict_size_delta;

    if (col == 0) {
      dict_size_main = 1;
      dict_size_delta = 1;
    } else {
      dict_size_main = main_size * distinct / 100;
      dict_size_delta = delta_size * distinct / 100;
    }

    std::vector<int64_t> *dict_values;
    dict_values = create_dicts(dict_size_main, dict_size_delta, 0, 0);

    // create main dict
    Progress p(dict_size_main);
    OrderPreservingDictionary<int64_t> *main_dict = new OrderPreservingDictionary<int64_t>;

    for (size_t j = 0; j < dict_size_main; ++j) {
      main_dict->addValue(dict_values->at(j));
      p.tick();
    }

    main->setDictionaryAt(AbstractTable::SharedDictionaryPtr(main_dict), col);

    // create delta dict
    Progress p1(dict_size_delta);
    OrderIndifferentDictionary<int64_t> *delta_dict = new OrderIndifferentDictionary<int64_t>;

    for (size_t j = 0; j < dict_size_delta; ++j) {
      delta_dict->addValue(dict_values->at(j + dict_size_main));
      p1.tick();
    }

    delta->setDictionaryAt(AbstractTable::SharedDictionaryPtr(delta_dict), col);

    delete dict_values;
  }

  main->resize(main_size);
  delta->resize(delta_size);

  for (size_t col = 0; col < cols; ++col) {

    // generate dict
    size_t distinct = col * 100 / (cols - 1);
    size_t dict_size_main;
    size_t dict_size_delta;

    if (col == 0) {
      dict_size_main = 1;
      dict_size_delta = 1;
    } else {
      dict_size_main = main_size * distinct / 100;
      dict_size_delta = delta_size * distinct / 100;
    }


    // set values in table
    Progress p2(main_size);

    for (size_t j = 0; j < main_size; ++j) {
      ValueId v;
      v.valueId = j % dict_size_main;
      v.table = 0;
      main->setValueId(col, j, v);
      p2.tick();
    }

    Progress p3(delta_size);

    for (size_t j = 0; j < delta_size; ++j) {
      ValueId v;
      v.valueId = j % dict_size_delta;
      v.table = 0;
      delta->setValueId(col, j, v);
      p3.tick();
    }

  }

  std::vector<atable_ptr_t > v;
  v.push_back(main);
  v.push_back(delta);
  return v;
}

std::vector<int64_t> *TableGenerator::create_dicts(size_t dict_size_main, size_t dict_size_delta, size_t intersection, size_t tail) {
  Progress p(dict_size_main + dict_size_delta);

  std::vector<int64_t> *v = new std::vector<int64_t>;
  v->reserve(dict_size_main + dict_size_delta);

  // maindict
  for (size_t i = 0; i < dict_size_main; ++i) {
    v->push_back(dict_size_delta + i);
    p.tick();
  }

  // deltadict
  size_t head = dict_size_delta - intersection - tail;

  for (size_t i = 0; i < head; ++i) {
    v->push_back(i);
    p.tick();
  }

  for (size_t i = head; i < head + intersection; ++i) {
    v->push_back(v->at(i - head));
    p.tick();
  }

  for (size_t i = head + intersection; i < dict_size_delta; ++i) {
    v->push_back(dict_size_main + dict_size_delta + i);
    p.tick();
  }

  v->at(dict_size_main + dict_size_delta - tail - 1) = v->at(dict_size_main - 1);

  return v;
}

std::vector<atable_ptr_t > TableGenerator::one_column_main_delta(size_t rows_main, size_t rows_delta, size_t dict_size_main, size_t dict_size_delta, size_t intersection, size_t tail) {
  std::vector<int64_t> *dict_values = create_dicts(dict_size_main, dict_size_delta, intersection, tail);
  TableGenerator table_generator;

  atable_ptr_t main = table_generator.create_empty_table(rows_main, 1);
  atable_ptr_t delta = table_generator.create_empty_table_modifiable(rows_delta, 1);

  Progress p(rows_main + rows_delta);

  std::set<int64_t> ordered_main;

  for (size_t i = 0; i < dict_size_main; ++i) {
    ordered_main.insert(dict_values->at(i));
  }

  OrderPreservingDictionary<int64_t> *main_dict = new OrderPreservingDictionary<int64_t>;

for (const auto & i: ordered_main) {
    main_dict->addValue(i);
  }

  main->setDictionaryAt(AbstractTable::SharedDictionaryPtr(main_dict), 0);


  // create delta dict
  Progress p1(dict_size_delta);
  OrderIndifferentDictionary<int64_t> *delta_dict = new OrderIndifferentDictionary<int64_t>;

  for (size_t j = 0; j < dict_size_delta; ++j) {
    delta_dict->addValue(dict_values->at(j + dict_size_main));
    p1.tick();
  }

  delta->setDictionaryAt(AbstractTable::SharedDictionaryPtr(delta_dict), 0);
  main->resize(rows_main);
  delta->resize(rows_main);

  for (size_t i = 0; i < rows_main; ++i) {
    ValueId v;
    v.valueId = i % dict_size_main;
    v.table = 0;
    main->setValueId(0, i, v);
    p.tick();
  }

  for (size_t i = 0; i < rows_delta; ++i) {

    ValueId v;
    v.valueId = i % dict_size_delta + dict_size_main;
    v.table = 0;
    delta->setValueId(0, i, v);
    p.tick();
  }

  std::vector<atable_ptr_t > tables;
  tables.push_back(main);
  tables.push_back(delta);
  return tables;
}




std::vector<atable_ptr_t > TableGenerator::value_order_successively(size_t rows_main, size_t rows_delta, size_t dict_size_main, size_t dict_size_delta, size_t intersection, size_t tail) {
  std::vector<int64_t> *dict_values = create_dicts(dict_size_main, dict_size_delta, intersection, tail);
  TableGenerator table_generator;

  atable_ptr_t main = table_generator.create_empty_table(rows_main, 1);
  atable_ptr_t delta = table_generator.create_empty_table_modifiable(rows_delta, 1);

  Progress p(rows_main + rows_delta);

  std::set<int64_t> ordered_main;

  for (size_t i = 0; i < dict_size_main; ++i) {
    ordered_main.insert(dict_values->at(i));
  }

  OrderPreservingDictionary<int64_t> *main_dict = new OrderPreservingDictionary<int64_t>;

for (const auto & i: ordered_main) {
    main_dict->addValue(i);
  }

  main->setDictionaryAt(AbstractTable::SharedDictionaryPtr(main_dict), 0);


  // create delta dict
  Progress p1(dict_size_delta);
  OrderIndifferentDictionary<int64_t> *delta_dict = new OrderIndifferentDictionary<int64_t>;

  for (size_t j = 0; j < dict_size_delta; ++j) {
    delta_dict->addValue(dict_values->at(j + dict_size_main));
    p1.tick();
  }

  delta->setDictionaryAt(AbstractTable::SharedDictionaryPtr(delta_dict), 0);
  main->resize(rows_main);
  delta->resize(rows_delta);

  for (size_t i = 0; i < rows_main; ++i) {
    ValueId vid;
    vid.table = 0;
    vid.valueId = i / (rows_main / dict_size_main);
    main->setValueId(0, i, vid);
    p.tick();
  }

  for (size_t i = 0; i < rows_delta; ++i) {
    ValueId vid;
    vid.table = 0;
    vid.valueId = i / (rows_delta / dict_size_delta) + dict_size_main;
    delta->setValueId(0, i, vid);
    p.tick();
  }

  std::vector<atable_ptr_t > v;
  v.push_back(main);
  v.push_back(delta);
  return v;
}



atable_ptr_t TableGenerator::create_empty_table(size_t rows, std::vector<std::string> names) {
  std::vector<std::vector<AbstractTable::SharedDictionaryPtr> *> dicts;
  std::vector<std::vector<ColumnMetadata > *> md;
  const auto& cols = names.size();
  for (size_t col = 0; col < cols; ++col) {
    std::vector<ColumnMetadata > *m = new std::vector<ColumnMetadata >;
    std::string colname(col < names.size() ? names.at(col) : "attr" + std::to_string(col)); 
    m->emplace_back(colname, IntegerType);
    md.push_back(m);

    auto d = new std::vector<AbstractTable::SharedDictionaryPtr>;
    auto new_dict = makeDictionary(IntegerType);
    d->push_back(new_dict);
    dicts.push_back(d);
  }

  auto new_table = std::make_shared<MutableVerticalTable>(md, &dicts, rows, false);

  for (const auto & d : dicts)
    delete d;
  
  for (const auto & m : md) {
      delete m;
  }
  new_table->resize(rows);
  return new_table;
}

atable_ptr_t TableGenerator::create_empty_table(size_t rows, size_t cols, std::vector<unsigned> containers) {
  std::vector<std::vector<ColumnMetadata > *> md;

  bool skip = containers.size() == 0 ? true : false;

  // Layout Handling
  unsigned currentCont = 0;
  unsigned currentSize = 0;
  unsigned contSize = skip ? cols : containers[0];

  std::vector<ColumnMetadata > *m = nullptr;

  for (size_t col = 0; col < cols; ++col) {
    if (currentSize == 0 || skip)
      m = new std::vector<ColumnMetadata >;

    std::stringstream s;
    s << "attr" << col;
    m->emplace_back(s.str(), IntegerType);


    if (++currentSize == contSize || skip) {
      md.push_back(m);
      currentSize = 0;

      if (++currentCont < containers.size() && !skip)
        contSize = containers[currentCont];
    }
  }

  auto new_table = std::shared_ptr<MutableVerticalTable>(new MutableVerticalTable(md, nullptr, rows, false));

for (const auto & vc: md) {
    delete vc;
  }

  new_table->reserve(rows);
  return new_table;
}

atable_ptr_t TableGenerator::create_empty_table_modifiable(size_t rows, size_t cols, std::vector<std::string> names) {
  std::vector<std::vector<AbstractTable::SharedDictionaryPtr> *> dicts;
  std::vector<std::vector<ColumnMetadata > *> md;

  for (size_t col = 0; col < cols; ++col) {
    std::vector<ColumnMetadata > *m = new std::vector<ColumnMetadata >;
    std::string colname(col < names.size() ? names.at(col) : "attr" + std::to_string(col)); 
    m->emplace_back(colname, IntegerTypeDelta);
    md.push_back(m);

    auto d = new std::vector<AbstractTable::SharedDictionaryPtr>;
    auto new_dict = makeDictionary(IntegerTypeDelta);
    d->push_back(new_dict);
    dicts.push_back(d);
  }

  auto new_table = std::make_shared<MutableVerticalTable>(md, &dicts, rows, false);

  for (const auto & d : dicts)
    delete d;
  
  for (const auto & m : md) {
    delete m;
  }
  new_table->resize(rows);
  return new_table;
}



atable_ptr_t TableGenerator::int_random_delta(size_t rows, size_t cols, size_t mod, std::vector<unsigned> layout) {

  start(rows, cols, rows * cols);
  unsigned seed = (unsigned) clock();


  atable_ptr_t new_table = create_empty_table(rows, cols, layout);


  for (size_t col = 0; col < cols; ++col) {

    OrderIndifferentDictionary<int64_t> *dict = new OrderIndifferentDictionary<int64_t>();

    // assume that we do not generate the same value twice...
    for (size_t row = 0; row < rows; ++row) {
      int r = rand_r(&seed);

      if (mod > 0) {
        r = r % mod;
      }

      dict->addValue(r);
      increment();
    }

    new_table->setDictionaryAt(AbstractTable::SharedDictionaryPtr(dict), col);
  }
  new_table->resize(rows);

  for (size_t col = 0; col < cols; ++col) {
    for (size_t row = 0; row < rows; ++row) {
      ValueId v;
      v.valueId = row;
      v.table = 0;
      new_table->setValueId(col, row, v);
    }
  }

  if (!_quiet) {
    std::cout << std::endl;
  }

  //new_table->sortDictionary();

  return new_table;

}

// create table with order preserving dict
atable_ptr_t TableGenerator::int_random(size_t rows, size_t cols, size_t mod, std::vector<unsigned> layout) {

  start(rows, cols, rows * cols);
  srand(clock());


  atable_ptr_t new_table = create_empty_table(rows, cols, layout);


  // if mod smaller than number of rows we have an infinite loop
  assert(mod > rows || mod == 0);

  prepare(rows);

  for (size_t col = 0; col < cols; ++col) {

    OrderPreservingDictionary<int64_t> *dict = new OrderPreservingDictionary<int64_t>();
    std::set<int64_t>::const_iterator it = _values.begin();
    for (size_t r = 0; r < rows; ++r) {
      if (mod == 0)
        dict->addValue(*(it++));
      else
        dict->addValue(r);
      increment();
    }

    new_table->setDictionaryAt(AbstractTable::SharedDictionaryPtr(dict), col);
  }

  // Resize the table correctly
  new_table->resize(rows);

  for (size_t col = 0; col < cols; ++col) {

    // shuffle the values
    std::vector<value_id_t> attribute_vector;

    for (size_t row = 0; row < rows; ++row) {
      attribute_vector.push_back(row);
    }

    random_shuffle(attribute_vector.begin(), attribute_vector.end());

    for (size_t row = 0; row < rows; ++row) {
      ValueId v;
      v.valueId = row;//attribute_vector[row];
      v.table = 0;
      new_table->setValueId(col, row, v);
    }
  }

  if (!_quiet) {
    std::cout << std::endl;
  }

  return new_table;

}


std::string TableGenerator::random_string(const int len) {
  static const char alphanum[] =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";

  std::string s;
  s.resize(len);

  unsigned seed = (unsigned) clock();
  for (int i = 0; i < len; ++i) {
    s[i] = alphanum[rand_r(&seed) % (sizeof(alphanum) - 1)];
  }

  return s;
}

atable_ptr_t TableGenerator::string_random(size_t rows, size_t cols, int string_length) {

  start(rows, cols, rows * cols);
  srand(clock());


  atable_ptr_t new_table = create_empty_table(rows, cols);

  for (size_t col = 0; col < cols; ++col) {
    std::set<std::string> values;

    for (size_t row = 0; row < rows; ++row) {
      values.insert(random_string(string_length));
      increment();
    }

    OrderPreservingDictionary<std::string> *dict = new OrderPreservingDictionary<std::string>();
for (const auto & i: values) {
      dict->addValue(i);
    }
    new_table->setDictionaryAt(AbstractTable::SharedDictionaryPtr(dict), col);
  }

  new_table->resize(rows);

  for (size_t col = 0; col < cols; ++col) {

    // shuffle the values
    std::vector<value_id_t> attribute_vector;
    for (size_t row = 0; row < rows; ++row) {
      attribute_vector.push_back(row % new_table->dictionaryAt(col)->size());
    }
    random_shuffle(attribute_vector.begin(), attribute_vector.end());

    for (size_t row = 0; row < rows; ++row) {
      ValueId v;
      v.valueId = attribute_vector[row];
      v.table = 0;
      new_table->setValueId(col, row, v);
    }
  }

  if (!_quiet) {
    std::cout << std::endl;
  }

  return new_table;
}

atable_ptr_t TableGenerator::string_random_delta(size_t rows, size_t cols, int string_length) {
  start(rows, cols, rows * cols);
  srand(clock());


  atable_ptr_t new_table = create_empty_table(rows, cols);


  for (size_t col = 0; col < cols; ++col) {
    std::set<std::string> values;

    for (size_t row = 0; row < rows; ++row) {
      values.insert(random_string(string_length));
      increment();
    }

    // shuffle the dict (its a delta)
    std::vector<std::string> values_vector(values.begin(), values.end());
    random_shuffle(values_vector.begin(), values_vector.end());

    OrderIndifferentDictionary<std::string> *dict = new OrderIndifferentDictionary<std::string>();
for (const auto & i: values_vector) {
      dict->addValue(i);
    }
    new_table->setDictionaryAt(AbstractTable::SharedDictionaryPtr(dict), col);

  }

  new_table->resize(rows);

  for (size_t col = 0; col < cols; ++col) {
    // shuffle the values
    std::vector<value_id_t> attribute_vector;

    for (size_t row = 0; row < rows; ++row) {
      attribute_vector.push_back(row % new_table->dictionaryAt(col)->size());
    }

    random_shuffle(attribute_vector.begin(), attribute_vector.end());

    for (size_t row = 0; row < rows; ++row) {
      ValueId v;
      v.valueId = attribute_vector[row];
      v.table = 0;
      new_table->setValueId(col, row, v);
    }
  }

  if (!_quiet) {
    std::cout << std::endl;
  }

  return new_table;
}

//vector<hyrise::storage::atable_ptr_t > TableGenerator::int_overlapping(size_t rows, size_t cols, size_t tables, size_t overlapping)
//{
//    std::vector<hyrise::storage::atable_ptr_t > r;
//
//    srand ( clock() );
//
//    assert(overlapping >= 0);
//    assert(overlapping <= 100);
//    assert(tables > 1);
//
//    hyrise::storage::atable_ptr_t ref_table = int_random(rows, cols);
//    r.push_back(ref_table);
//
//
//    std::cout << "second table" << std::endl;
//    for (size_t i = 0; i < tables - 1; ++i)
//    {
//        hyrise::storage::atable_ptr_t new_table = create_empty_table(rows, cols);
//        start(rows, cols, rows * cols * (tables-1));
//
//        for (size_t col = 0; col < cols; ++col) {
//            OrderIndifferentDictionary<int64_t>* dict = new OrderIndifferentDictionary<int64_t>();
//            new_table->setDictionaryAt(dict, col);
//
//            for (size_t row = 0; row < rows; ++row)
//            {
//                int value;
//                if ((size_t)(rand() % 100) < overlapping)
//                    value = ref_table->getValue<int64_t>(col, row);
//                else
//                    value = rand();
//                dict->addValue(value);
//
//                ValueId v;
//                v.valueId = row;
//                v.table = 0;
//                new_table->setValueId(col, row, v);
//
//                increment();
//            }
//        }
//        r.push_back(new_table);
//    }
//
//    return r;
//}

atable_ptr_t TableGenerator::int_offset(size_t rows, size_t cols, size_t offset1, size_t offset2, size_t offset2_start, size_t factor, int big_value_at_end) {
  start(rows, cols, rows * cols);
  srand(clock());

  atable_ptr_t new_table = create_empty_table(rows, cols);

  for (size_t col = 0; col < cols; ++col) {
    OrderPreservingDictionary<int64_t> *dict = new OrderPreservingDictionary<int64_t>();
    new_table->setDictionaryAt(AbstractTable::SharedDictionaryPtr(dict), col);
    for (size_t row = 0; row < rows; ++row) {
      int value = row * factor + offset1;

      if (row >= offset2_start) {
        value += offset2;
      }

      if (big_value_at_end > 0 && row == rows - 1) {
        value = big_value_at_end;
      }

      dict->addValue(value);
    }
  }

  new_table->resize(rows);

  for (size_t col = 0; col < cols; ++col) {
    for (size_t row = 0; row < rows; ++row) {

      ValueId v;
      v.valueId = row;
      v.table = 0;
      new_table->setValueId(col, row, v);

      increment();
    }
  }

  return new_table;

}

atable_ptr_t TableGenerator::int_offset_delta(size_t rows, size_t cols, size_t offset1, size_t offset2, size_t offset2_start, size_t factor, int big_value_at_end) {
  start(rows, cols, rows * cols);
  srand(clock());

  atable_ptr_t new_table = create_empty_table(rows, cols);

  for (size_t col = 0; col < cols; ++col) {
    OrderIndifferentDictionary<int64_t> *dict = new OrderIndifferentDictionary<int64_t>();
    new_table->setDictionaryAt(AbstractTable::SharedDictionaryPtr(dict), col);
    for (size_t row = 0; row < rows; ++row) {
      int value = row * factor + offset1;

      if (row >= offset2_start) {
        value += offset2;
      }

      if (big_value_at_end > 0 && row == rows - 1) {
        value = big_value_at_end;
      }

      dict->addValue(value);
    }
  }

  new_table->resize(rows);

  for (size_t col = 0; col < cols; ++col) {
    for (size_t row = 0; row < rows; ++row) {
      ValueId v;
      v.valueId = row;
      v.table = 0;
      new_table->setValueId(col, row, v);

      increment();
    }
  }

  return new_table;

}

atable_ptr_t TableGenerator::one_value_delta(size_t rows, size_t cols, int value) {
  start(rows, cols, rows * cols);
  srand(clock());

  atable_ptr_t new_table = create_empty_table(rows, cols);

  for (size_t col = 0; col < cols; ++col) {
    OrderIndifferentDictionary<int64_t> *dict = new OrderIndifferentDictionary<int64_t>();
    new_table->setDictionaryAt(AbstractTable::SharedDictionaryPtr(dict), col);

    dict->addValue(value);
  }
  new_table->resize(rows);

  for (size_t col = 0; col < cols; ++col) {
    for (size_t row = 0; row < rows; ++row) {
      ValueId v;
      v.valueId = 0;
      v.table = 0;
      new_table->setValueId(col, row, v);

      increment();
    }
  }

  return new_table;
}

atable_ptr_t TableGenerator::one_value(size_t rows, size_t cols, int value) {
  start(rows, cols, rows * cols);
  srand(clock());

  atable_ptr_t new_table = create_empty_table(rows, cols);

  for (size_t col = 0; col < cols; ++col) {
    OrderPreservingDictionary<int64_t> *dict = new OrderPreservingDictionary<int64_t>();
    new_table->setDictionaryAt(AbstractTable::SharedDictionaryPtr(dict), col);

    dict->addValue(value);
  }
  new_table->resize(rows);
  for (size_t col = 0; col < cols; ++col) {
    for (size_t row = 0; row < rows; ++row) {
      ValueId v;
      v.valueId = 0;
      v.table = 0;
      new_table->setValueId(col, row, v);

      increment();
    }
  }

  return new_table;
}

// creates one table with distinct_count distinct values
atable_ptr_t TableGenerator::int_distinct(size_t row_count, size_t column_count, size_t distinct_count) {
  size_t x = row_count / distinct_count;
  std::vector<c_atable_ptr_t > tables;
  const auto& base_table = int_random(distinct_count, column_count);
  tables.push_back(base_table);

  for (size_t i = 0; i <= x; i++) {
    tables.push_back(base_table);
  }

  size_t fill_up = row_count - x * distinct_count;

  if (fill_up > 0) {
    const auto& fill_up_table = int_random(fill_up, column_count);
    tables.push_back(fill_up_table);
  }

  const auto& result = TableMerger(new DefaultMergeStrategy(), new SequentialHeapMerger()).merge(tables);
  return result[0];
}

int TableGenerator::selfsimilar(int64_t n, double h) {
  unsigned seed  = (unsigned) clock();
  return (int)(n * pow(rand_r(&seed), log(h) / log(1.0 - h)));
}

atable_ptr_t TableGenerator::int_random_weighted(size_t rows, size_t cols, size_t n, size_t h) {
  unsigned seed = (unsigned) clock();

  atable_ptr_t new_table = create_empty_table(rows, cols);

  for (size_t col = 0; col < cols; ++col) {
    std::set<int64_t> values;

    while (values.size() < n) {
      int r = rand_r(&seed);

      values.insert(r);
    }

    OrderPreservingDictionary<int64_t> *dict = new OrderPreservingDictionary<int64_t>();
for (const auto & i: values) {
      dict->addValue(i);
    }
    new_table->setDictionaryAt(AbstractTable::SharedDictionaryPtr(dict), col);
  }

  new_table->resize(rows);

  for (size_t col = 0; col < cols; ++col) {
    for (size_t row = 0; row < rows; ++row) {
      ValueId v;
      v.valueId = selfsimilar(n, h);
      v.table = 0;
      new_table->setValueId(col, row, v);
    }
  }

  if (!_quiet) {
    std::cout << std::endl;
  }

  return new_table;
}

atable_ptr_t TableGenerator::int_random_weighted_delta(size_t rows, size_t cols, size_t n, size_t h) {
  unsigned seed = (unsigned) clock();

  atable_ptr_t new_table = create_empty_table(rows, cols);

  for (size_t col = 0; col < cols; ++col) {

    OrderIndifferentDictionary<int64_t> *dict = new OrderIndifferentDictionary<int64_t>();

    while (dict->size() < n) {
      int r = rand_r(&seed);

      dict->addValue(r);
    }

    new_table->setDictionaryAt(AbstractTable::SharedDictionaryPtr(dict), col);
  }
  new_table->resize(rows);
  for (size_t col = 0; col < cols; ++col) {
    for (size_t row = 0; row < rows; ++row) {
      ValueId v;
      v.valueId = selfsimilar(n, h);
      v.table = 0;
      new_table->setValueId(col, row, v);
    }
  }

  if (!_quiet) {
    std::cout << std::endl;
  }

  return new_table;
}


}}

