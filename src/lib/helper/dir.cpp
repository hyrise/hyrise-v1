// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "dir.h"

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <iostream>
#include <stdexcept>
#include <unistd.h>

void _mkdir(const std::string dir) {
  struct stat sb;
  if (stat(dir.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)) {
    return;
  }

  char tmp[512];
  char* p = NULL;
  size_t len;

  snprintf(tmp, sizeof(tmp), "%s", dir.c_str());
  len = strlen(tmp);
  if (tmp[len - 1] == '/')
    tmp[len - 1] = 0;
  for (p = tmp + 1; *p; p++)
    if (*p == '/') {
      *p = 0;
      mkdir(tmp, S_IRWXU);
      *p = '/';
    }
  mkdir(tmp, S_IRWXU);

  if (stat(dir.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)) {
    return;
  } else {
    throw std::runtime_error("Could not create directory: " + dir);
  }
}

std::vector<std::string> _listdir(std::string path_name) {
  // Files Vector
  std::vector<std::string> files;
  // Open Directory
  DIR* dp = opendir(path_name.c_str());

  // While Directory is Opened
  while (dp != NULL) {
    // Open Node
    dirent* np = readdir(dp);

    // If No Nodes, Close Directory and Break
    if (np == NULL) {
      closedir(dp);
      break;
    }

    // Create String for Node Name
    std::string node_name(np->d_name);

    // Add Node to Vector
    if (node_name != "." && node_name != "..")
      files.push_back(node_name);
  }
  return files;
}

std::string _current_working_dir() {
  char cwd[1024];
  if (getcwd(cwd, sizeof(cwd)) == NULL)
    throw std::runtime_error("Could not get CWD.");
  return std::string(cwd);
}
