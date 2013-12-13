// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "io/GenericCSV.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <istream>
#include <stdexcept>
#include <memory>

#include <string.h>
#include <libcsv/csv.h>

namespace hyrise {
namespace io {

param_member_impl(csv::params, unsigned char, Delimiter);
param_member_impl(csv::params, ssize_t, LineStart);
param_member_impl(csv::params, ssize_t, LineCount); // -1 means unlimited

namespace csv {

void genericLineBasedParsing(
    std::istream &file,
    field_cb_t cb_per_field,
    line_cb_t cb_per_line,
    void *data,
    const csv::params &params) {
  struct csv_parser parser;

  if (!csv_init(&parser, 0)) {
    csv_set_opts(&parser, CSV_APPEND_NULL);
    csv_set_delim(&parser, params.getDelimiter());

    std::string line;
    int line_start = params.getLineStart();

    if (line_start != 1) {
      while (line_start > 1) {
        std::getline(file, line);
        --line_start;
      }
    }


    int lineCount = 0;
    while (std::getline(file, line)) {

      ++lineCount;
      line.append("\n");
      if (csv_parse(&parser,
                    line.c_str(),
                    line.size(),
                    cb_per_field,
                    cb_per_line,
                    data) != line.size()) {
        throw ParserError(csv_strerror(csv_error(&parser)));
      }

      if (params.getLineCount() != -1 && lineCount >= params.getLineCount())
        break;

      if (file.bad())
        break;
    }

    csv_fini(&parser,
             cb_per_field,
             cb_per_line,
             data);
  }
  csv_free(&parser);

}

void genericParse(
    /*std::istream &file,*/
    std::string filename,
    field_cb_t cb_per_field,
    line_cb_t cb_per_line,
    void *data,
    const csv::params &params
                  ) {
  // Open the file
  typedef std::unique_ptr<std::FILE, int (*)(std::FILE *)> unique_file_ptr;
  unique_file_ptr file(fopen(filename.c_str(), "rb"), fclose);
  if (!file) {
    throw ParserError(std::string("File Opening Failed") +  std::strerror(errno));
  }

  struct csv_parser parser;

  if (!csv_init(&parser, 0)) {
    csv_set_opts(&parser, CSV_APPEND_NULL);
    csv_set_delim(&parser, params.getDelimiter());

    int line_start = params.getLineStart();
    if (line_start > 1) {
      int c;
      do {
        c = fgetc(file.get());
        if ( c== '\n') --line_start;
      } while (c!= EOF  && line_start > 1);
    }

    // 1GB Buffer
    size_t block_size;
    if (getenv("HYRISE_LOAD_BLOCK_SIZE"))
      block_size = strtoul(getenv("HYRISE_LOAD_BLOCK_SIZE"), nullptr, 0);
    else
      block_size = 1024 * 1024;

    // Read from the buffer
    size_t readBytes = 0;
    char rdbuf[block_size];

    // Read the file until we cannot extract more bytes
    do {
      readBytes = fread(rdbuf, 1, block_size, file.get());
      if (csv_parse(&parser,
                    rdbuf,
                    readBytes,
                    cb_per_field,
                    cb_per_line,
                    data) != (size_t) readBytes) {
        throw ParserError(csv_strerror(csv_error(&parser)));
      }
    } while (readBytes == block_size);

    if (ferror(file.get())) {
      throw ParserError("Could not read file");
    }

    csv_fini(&parser,
             cb_per_field,
             cb_per_line,
             data);
  }
  csv_free(&parser);
}

void genericParseFile(
    const std::string &filename,
    field_cb_t cb_per_field,
    line_cb_t cb_per_line,
    void *data,
    const csv::params &params
                      ) {

  if (params.getLineCount() > 0) {
      std::ifstream file(filename.c_str(), std::ios::binary);
      if (!file || file.bad()) {
        throw ParserError("CSV file '" + filename + "' does not exist");
      }
      genericLineBasedParsing(file, cb_per_field, cb_per_line, data, params);
      return;
  }

  genericParse(filename, cb_per_field, cb_per_line, data, params);
}

void vector_cb_per_field(char *field_buffer, size_t field_length, struct vector_cb_data *data) {
  std::string content(field_buffer, field_length);
  data->lines[data->lines.size() - 1].push_back(content);
}

void vector_cb_per_line(int separator, struct vector_cb_data *data) {
  data->lines.resize(data->lines.size() + 1);
}

std::vector< line_t> parse_stream(std::istream &stream, const params &params) {
  vector_cb_data data;
  genericLineBasedParsing(stream, (field_cb_t) vector_cb_per_field, (line_cb_t) vector_cb_per_line, &data, params);
  return data.lines;
}

std::vector< line_t > parse_file(const std::string &filename, const params &params) {
  vector_cb_data data;
  genericParseFile(filename, (field_cb_t) vector_cb_per_field, (line_cb_t) vector_cb_per_line, &data, params);
  return data.lines;
}

} } } // namespace hyrise::io::csv

