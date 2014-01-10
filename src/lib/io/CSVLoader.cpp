// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "io/CSVLoader.h"

#include <fstream>

#include "io/GenericCSV.h"
#include "io/MetadataCreation.h"
#include "storage/AbstractTable.h"
#include "storage/ColumnMetadata.h"

namespace hyrise {
namespace io {

param_member_impl(CSVInput::params, csv::params, CSVParams);
param_member_impl(CSVInput::params, bool, Unsafe);
param_member_impl(CSVHeader::params, csv::params, CSVParams);

struct cb_data {
  size_t row;
  size_t column;
  const size_t table_columns;
  const bool unsafe;
  std::shared_ptr<storage::AbstractTable> table;

  cb_data(size_t columns, bool unsafe): row(0), column(0), table_columns(columns), unsafe(unsafe) {
  }
};


void cb_per_field(char *field_buffer, size_t field_length, struct cb_data *data) {
  if (data->column >= data->table_columns) {
    if (data->unsafe) goto ignore_data;
    else throw CSVLoaderError("There is more data than columns!");
  }
  switch (data->table->typeOfColumn(data->column)) {
  case IntegerType:
  case IntegerTypeDelta:
  case IntegerTypeDeltaConcurrent:
    data->table->setValue<hyrise_int_t>(data->column, data->row, atol(field_buffer));
    break;
  case IntegerNoDictType:
    data->table->setValue<hyrise_int32_t>(data->column, data->row, atoi(field_buffer));
    break;
    
  case FloatType:
  case FloatTypeDelta:
  case FloatTypeDeltaConcurrent:
  case FloatNoDictType:
    data->table->setValue<hyrise_float_t>(data->column, data->row, atof(field_buffer));
    break;

  case StringType:
  case StringTypeDelta:
  case StringTypeDeltaConcurrent:
    data->table->setValue<hyrise_string_t>(data->column, data->row, std::string(field_buffer));
    break;

  default:
    throw std::runtime_error("FUUUU");
    break;
  }
ignore_data:
  ++data->column;
}

void cb_per_line(int separator, struct cb_data *data) {
  if ((!data->unsafe) && (data->column != data->table_columns)) {
    throw CSVLoaderError("Less data than columns");
  }
  data->row++;
  data->column = 0;
}

uint64_t countLines(const std::string &filename) {
  int numLines = 0;
  std::ifstream in(filename);
  std::string l;
  while (std::getline(in, l))
    ++numLines;

  //  if (l.size() == 0)
  //  --numLines;

  return numLines;
}

bool detectHeader(const std::string &filename) {
  // Take a peak into file to check wether it features a header
  std::ifstream file(filename.c_str());
  if (file.fail()) {
    throw CSVLoaderError("File '" + filename + "' does not exist");
  }
  std::string line;
  for (int i = 0; i < 4; ++i) getline(file, line);
  file.close();
  return "===" == line;
}

std::shared_ptr<storage::AbstractTable> CSVInput::load(std::shared_ptr<storage::AbstractTable> intable, const storage::compound_metadata_list *meta, const Loader::params &args) {
  cb_data data(intable->columnCount(), _parameters.getUnsafe());
  data.table = intable;
  csv::params params(_parameters.getCSVParams());

  if (detectHeader(args.getBasePath() + _filename)) params.setLineStart(5);

  // Resize the table based on the file size
  data.table->resize(countLines(args.getBasePath() + _filename) - params.getLineStart() + 1);

  try {
    csv::genericParseFile(args.getBasePath() + _filename,
                          (field_cb_t) cb_per_field,
                          (line_cb_t) cb_per_line,
                          &data,
                          params);
  } catch (const csv::ParserError &e) {
    throw Loader::Error(e.what());
  }

  data.table->resize(data.row);
  return intable;
}

CSVInput *CSVInput::clone() const {
  return new CSVInput(*this);
}

CSVHeader *CSVHeader::clone() const {
  return new CSVHeader(*this);
}

storage::compound_metadata_list *CSVHeader::load(const Loader::params &args) {
  csv::params p(_parameters.getCSVParams());
  p.setLineCount(4);
  try {
    std::vector<line_t> lines(csv::parse_file(args.getBasePath() + _filename, p));
    return createMetadata(lines);
  } catch (std::runtime_error &e) {
    throw Loader::Error(e.what());
  }

}

} } // namespace hyrise::io

