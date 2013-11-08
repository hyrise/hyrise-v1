// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "HttpHelper.h"

#include <sstream>
#include <vector>

#include <boost/algorithm/string.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


namespace test {

// http://www.geekhideout.com/urlcode.shtml

char from_hex(char ch) {
  return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

/* Converts an integer value to its hex character*/
char to_hex(char code) {
  static char hex[] = "0123456789abcdef";
  return hex[code & 15];
}

/* Returns a url-decoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
std::unique_ptr<char, decltype(&std::free)> url_decode(const char *str) {
  const char *pstr = str;
  char *buf = (char*) malloc(strlen(str) + 1), *pbuf = buf;
  while (*pstr) {
    if (*pstr == '%') {
      if (pstr[1] && pstr[2]) {
        *pbuf++ = from_hex(pstr[1]) << 4 | from_hex(pstr[2]);
        pstr += 2;
      }
    } else if (*pstr == '+') {
      *pbuf++ = ' ';
    } else {
      *pbuf++ = *pstr;
    }
    pstr++;
  }
  *pbuf = '\0';
  return {buf, &std::free};
}

}


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
  auto t = test::url_decode(input.c_str());
  std::string res(t.get());
  return std::move(res);
}
