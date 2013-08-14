// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "HttpHelper.h"

#include <sstream>
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

std::string urldecode(const std::string &input) {
  std::ostringstream escaped;
  for (auto i = 0ul, len = input.length(); i < len; ++i) {
    switch (char ch = input[i]) {
      case '%':
        int chnum;
        sscanf(input.substr(i+1, 2).c_str(), "%x", &chnum);
        escaped << static_cast<char>(chnum);
        i += 2;
        break;
      case '+':
        escaped << " "; break;
      default:
        escaped << ch;
    }
  }
  return escaped.str();
}
