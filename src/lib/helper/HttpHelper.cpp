// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "HttpHelper.h"
#include <vector>
#include <boost/algorithm/string.hpp>



std::map<std::string, std::string> parseHTTPFormData(std::string formData, const std::string elem_sep) {
  typedef boost::split_iterator<std::string::iterator> string_split_iterator;
  typedef std::vector<std::string> split_v_t;

  std::map<std::string, std::string> result;

  // Iterate over all elements
  for (string_split_iterator it = boost::make_split_iterator(formData,
                                                             boost::first_finder(elem_sep, boost::is_equal()));
       it != string_split_iterator();
       ++it) {
    // *it points to the element std::string that needs to be splited into two parts
    split_v_t elements;
    boost::split(elements, *it, boost::is_any_of("="));
    result.insert(std::pair<std::string, std::string>(elements.front(), elements.back()));
  }

  return result;
}

std::string rawurldecode(const std::string &input) {
  std::string buffer = "";
  int len = input.length();

  for (int i = 0; i < len; i++) {
    int j = i ;
    char ch = input.at(j);
    if (ch == '%') {
      char tmpstr[] = "0x0__";
      int chnum;
      tmpstr[3] = input.at(j + 1);
      tmpstr[4] = input.at(j + 2);
      chnum = strtol(tmpstr, nullptr, 16);
      buffer += chnum;
      i += 2;
    } else {
      buffer += ch;
    }
  }
  return buffer;
}

std::string urldecode(const std::string &input) {
  auto tmp(input);
  std::replace(tmp.begin(), tmp.end(), '+', ' ');
  std::string buf = rawurldecode(tmp);
  return buf;
}
