// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <map>
#include <string>

std::map<std::string, std::string> parseHTTPFormData(std::string formData, const std::string elem_sep = "&");

std::string urldecode(const std::string &input);

