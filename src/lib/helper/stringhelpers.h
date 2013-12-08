// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <string>
#include <sstream>


void inline splitString(std::vector<std::string> &result,
                        const std::string &s,
                        const std::string &delim) {
  std::stringstream ss(s);
  std::string item;
  char cdelim = delim[0];

  while (std::getline(ss, item, cdelim)) {
    result.push_back(item);
  }
}

template<class T>
std::string inline toString(T s) {
  std::stringstream sstm;
  sstm << s;
  return sstm.str();
}

template <typename T>
std::string inline joinString(const std::vector<T> &items, std::string connector) {
  std::stringstream ss;
  for (size_t i = 0; i < items.size(); ++i) {
    if (i != 0) ss << connector;
    ss << toString(items[i]);
  }
  return ss.str();
}

template<class T>
T inline fromString(const std::string &s) {
  std::istringstream stream(s);
  T t;
  stream >> t;
  return t;
}

struct infix {
  std::string sep;
  infix(const std::string& sep) : sep(sep) {}

  inline std::string operator()(const std::string& lhs, const std::string& rhs) {
    std::string rz(lhs);
    if(!lhs.empty() && !rhs.empty())
      rz += sep;
    rz += rhs;
    return rz;
  }
};

