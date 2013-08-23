// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "io/shortcuts.h"

#include "io/loaders.h"
#include "storage/AbstractTable.h"
#include "storage/Store.h"

namespace Loader {
namespace shortcuts {
Loader::params loadParams(const std::string &filename, Loader::params &p) {
  CSVInput input(filename);
  CSVHeader header(filename);

  p.setInput(input);
  p.setHeader(header);
  return p;
}

Loader::params loadWithHeaderParams(const std::string &datafilepath, const std::string &headerfilepath) {
  CSVInput input(datafilepath, CSVInput::params().setUnsafe(true));
  CSVHeader header(headerfilepath);
  Loader::params p;
  p.setReturnsMutableVerticalTable(true);
  p.setInput(input);
  p.setHeader(header);
  return p;

}

Loader::params loadWithStringHeaderParams(const std::string &datafilepath, const std::string &header) {
  CSVInput input(datafilepath);
  StringHeader sheader(header);
  Loader::params p;
  p.setInput(input);
  p.setHeader(sheader);
  return p;
}

}
}

std::shared_ptr<AbstractTable> Loader::shortcuts::load(const std::string &filepath) {
  Loader::params p;
  return Loader::load(loadParams(filepath, p));
};

std::shared_ptr<AbstractTable> Loader::shortcuts::load(const std::string &filepath, Loader::params &params) {
  return Loader::load(loadParams(filepath, params));
};

std::shared_ptr<AbstractTable> Loader::shortcuts::loadWithHeader(const std::string &datafilepath, const std::string &headerfilepath) {
  return Loader::load(loadWithHeaderParams(datafilepath, headerfilepath));
};

std::shared_ptr<AbstractTable> Loader::shortcuts::loadWithStringHeader(const std::string &datafilepath, const std::string &header) {
  return Loader::load(loadWithStringHeaderParams(datafilepath, header));
};

std::shared_ptr<hyrise::storage::Store> Loader::shortcuts::loadMainDelta(const std::string &mainfilepath, const std::string &deltafilepath, Loader::params p) {
  std::vector<std::string> filenames;
  filenames.push_back(mainfilepath);
  filenames.push_back(deltafilepath);
  std::vector<std::shared_ptr<AbstractTable>> tables;

  for (int i = 0; i < 2; ++i) {
    CSVInput input(filenames[i]);
    CSVHeader header(filenames[i]);

    p.setInput(input);
    p.setHeader(header);
    p.setReturnsMutableVerticalTable(true);
    std::shared_ptr<AbstractTable> table = Loader::load(p);
    tables.push_back(table);
  }
  auto s = std::make_shared<hyrise::storage::Store>(tables[0]);
  s->setDelta(tables[1]);
  return s;
};

std::shared_ptr<AbstractTable> Loader::shortcuts::loadRaw(const std::string &file) {
  hyrise::io::RawTableLoader input(file);
  CSVHeader header(file);

  Loader::params p;
  p.setInput(input);
  p.setHeader(header);
  p.setReturnsMutableVerticalTable(false);
  return Loader::load(p);
}
