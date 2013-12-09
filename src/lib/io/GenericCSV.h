// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <iosfwd>
#include <stdexcept>
#include <string>
#include <vector>

#include <libcsv/csv.h>

#include "storage/storage_types.h"

namespace hyrise {
namespace io {

typedef std::vector<std::string> line_t;
typedef void (*field_cb_t)(void *, size_t, void *);
typedef void (*line_cb_t)(int, void *);

namespace csv {
class ParserError : public std::runtime_error {
 public:
  explicit ParserError(const std::string &what): std::runtime_error(what)
  {}
};

class params {
 public:
#include "parameters.inc"
  param_member(unsigned char, Delimiter);
  param_member(ssize_t, LineStart);
  param_member(ssize_t, LineCount); // -1 means unlimited
  params() : Delimiter('|'), LineStart(0), LineCount(-1) {}
};

const params HYRISE_FORMAT(params().setDelimiter('|'));
const params HYRISE_HEADER(params().setDelimiter('|').setLineCount(3));
const params CSV_FORMAT(params().setDelimiter(CSV_COMMA));

/**
 * Generically parsing an std::istream with libcsv
 */
void genericParse(
    /*std::istream &filename,*/
    std::string filename,
    field_cb_t field_callback,
    line_cb_t line_callback,
    void *cb_data,
    const csv::params &params = csv::params());

void genericParseFile(
    const std::string &filename,
    field_cb_t field_callback,
    line_cb_t line_callback,
    void *cb_data,
    const csv::params &params = csv::params());

std::vector<line_t> parse_file(const std::string &filename, const csv::params &params = csv::params());
std::vector<line_t> parse_stream(std::istream &instream, const csv::params &params = csv::params());

struct vector_cb_data {
  std::vector< line_t > lines;
  vector_cb_data() {
    lines.resize(1);
  }
};

void vector_cb_per_field(char *field_buffer, size_t field_length, struct vector_cb_data *data);
void vector_cb_per_line(int separator, struct vector_cb_data *data);


} //namespace csv

} } // namespace hyrise::io

