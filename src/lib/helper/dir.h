// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <string>
#include <vector>

void _mkdir(const std::string dir);
std::vector<std::string> _listdir(std::string path_name);
std::string _current_working_dir();
