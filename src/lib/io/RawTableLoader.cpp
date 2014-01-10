// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "RawTableLoader.h"
#include "CSVLoader.h"
#include "GenericCSV.h"

#include <fstream>

#include <storage/RawTable.h>

namespace hyrise { namespace io {

struct raw_table_cb_data {
  std::shared_ptr<storage::RawTable> table;
  storage::rawtable::RowHelper rh;
  size_t column;

  raw_table_cb_data(const storage::metadata_vec_t& meta) : rh(meta), column(0) {
  }
};

void raw_table_cb_per_field(char* field_buffer, size_t field_length, struct raw_table_cb_data *data) {
  switch(data->table->typeOfColumn(data->column)) {
  case IntegerType:
  case IntegerTypeDelta:
  case IntegerTypeDeltaConcurrent:
  case IntegerNoDictType:
    data->rh.set<hyrise_int_t>(data->column, atol(field_buffer));
    break;
  case StringType:
  case StringTypeDelta:
  case StringTypeDeltaConcurrent:
    data->rh.set<hyrise_string_t>(data->column, std::string(field_buffer, field_length));
    break;
  case FloatType:
  case FloatTypeDelta:
  case FloatTypeDeltaConcurrent:
  case FloatNoDictType:
    data->rh.set<hyrise_float_t>(data->column, atof(field_buffer));
    break;
  default:
    throw std::runtime_error("Type not supported");
  }
  data->column++;
}

void raw_table_cb_per_line(int separator, struct raw_table_cb_data *data) {
  auto tmp = data->rh.build();
  data->table->appendRow(tmp);
  free(tmp);
  data->column = 0;
  data->rh.reset();
}


std::shared_ptr<storage::AbstractTable> RawTableLoader::load(std::shared_ptr<storage::AbstractTable> in,
                                                             const storage::compound_metadata_list *ml,
                                                             const Loader::params &args) {



  csv::params params;
  if (detectHeader(args.getBasePath() + _filename)) params.setLineStart(5);

  // Create the result table
  storage::metadata_vec_t v(in->columnCount());
  for(size_t i=0; i < in->columnCount(); ++i) {
    v[i] = in->metadataAt(i);
  }
  auto result = std::make_shared<storage::RawTable>(v);

  // CSV Parsing
  std::ifstream file(args.getBasePath() + _filename, std::ios::binary);
  if (!file || file.bad()) {
    throw csv::ParserError("CSV file '" + _filename + "' does not exist");
  }

  struct csv_parser parser;

  if (!csv_init(&parser, 0)) {
    csv_set_opts(&parser, CSV_APPEND_NULL);
    csv_set_delim(&parser, params.getDelimiter());

    // If there is a header in the file, we will ignore it
    std::string line;
    int line_start = params.getLineStart();

    if (line_start != 1) {
      while (line_start > 1) {
        std::getline(file, line);
        --line_start;
      }
    }

    // Prepare cb data handler
    struct raw_table_cb_data data(v);
    data.table = result;

    const size_t block_size = 16 * 1024;
    char rdbuf [block_size];

    while (file.read(rdbuf, block_size).good()) {
      auto extracted = file.gcount();
      if (extracted == 0)
        break;

      if (csv_parse(&parser,
                    rdbuf,
                    extracted,
                    (field_cb_t) raw_table_cb_per_field,
                    (line_cb_t) raw_table_cb_per_line,
                    (void*) &data) != (size_t) extracted) {
        throw csv::ParserError(csv_strerror(csv_error(&parser)));
      }
    }

    // Parse the rest
    if (csv_parse(&parser,
                  rdbuf,
                  file.gcount(),
                  (field_cb_t) raw_table_cb_per_field,
                  (line_cb_t) raw_table_cb_per_line,
                  (void*) &data) != (size_t) file.gcount()) {
      throw csv::ParserError(csv_strerror(csv_error(&parser)));
    }

    csv_fini(&parser,
             (field_cb_t) raw_table_cb_per_field,
             (line_cb_t) raw_table_cb_per_line,
             (void*) &data);

  }
  csv_free(&parser);
  return result;
}

}}

