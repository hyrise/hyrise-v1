// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "helper.h"

#include <fstream>

std::string loadFromFile(std::string path) {
  std::ifstream data_file(path.c_str());
  std::string result((std::istreambuf_iterator<char>(data_file)), std::istreambuf_iterator<char>());
  data_file.close();
  return result;
}
