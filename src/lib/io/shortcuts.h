// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_IO_SHORTCUTS_H_
#define SRC_LIB_IO_SHORTCUTS_H_

#include <memory>
#include <string>

#include "Loader.h"

class Store;
class AbstractTable;

namespace Loader {

namespace shortcuts {

Loader::params loadWithStringHeaderParams(const std::string &datafilepath, const std::string &header);

/*! Loads a table for a given filename, assuming it contains header and data in HYRISE format.
  @param filepath
*/
std::shared_ptr<AbstractTable> load(const std::string &filepath);
std::shared_ptr<AbstractTable> load(const std::string &filepath, Loader::params &params);

/*! Load a table from a given filename, while supplying the header from another file. Must be HYRISE formatted.
  @param datafilepath
  @param headerfilepath
*/
std::shared_ptr<AbstractTable> loadWithHeader(const std::string &datafilepath, const std::string &headerfilepath);

/*! Load a table from a given filename, while supplying the header from the second input parameter. Must be HYRISE formatted.
  @param datafilepath
  @param header
*/
std::shared_ptr<AbstractTable> loadWithStringHeader(const std::string &datafilepath, const std::string &header);

/*! Load store with main and delta from separate files
  @param mainfilepath
  @param deltafilepath
*/
std::shared_ptr<Store> loadMainDelta(const std::string &mainfilepath, const std::string &deltafilepath, Loader::params p = Loader::params());

/*! Load insert only
  @param datafilepath
  @param headerfilepath
*/
std::shared_ptr<AbstractTable> loadInsertOnly(const std::string &datafilepath, const std::string &headerfilepath);

/*! Loader insert only, deprecated structure */
std::shared_ptr<AbstractTable> loadInsertOnlyDeprecated(const std::string &datafilepath, const std::string &headerfilepath);

/**
 * Load a table into a raw table container
 */
std::shared_ptr<AbstractTable> loadRaw(const std::string &file);

}} // end namespace Loader::shortcuts

#endif  // SRC_LIB_IO_SHORTCUTS_H_
