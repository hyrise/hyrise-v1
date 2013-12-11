// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <memory>
#include <string>

#include "helper/types.h"
#include "Loader.h"

namespace hyrise {
namespace io {
namespace Loader {
namespace shortcuts {

params loadWithStringHeaderParams(const std::string &datafilepath, const std::string &header);

/*! Loads a table for a given filename, assuming it contains header and data in HYRISE format.
  @param filepath
*/
std::shared_ptr<storage::AbstractTable> load(const std::string &filepath);
std::shared_ptr<storage::AbstractTable> load(const std::string &filepath, params &params);

/*! Load a table from a given filename, while supplying the header from another file. Must be HYRISE formatted.
  @param datafilepath
  @param headerfilepath
*/
std::shared_ptr<storage::AbstractTable> loadWithHeader(const std::string &datafilepath, const std::string &headerfilepath);

/*! Load a table from a given filename, while supplying the header from the second input parameter. Must be HYRISE formatted.
  @param datafilepath
  @param header
*/
std::shared_ptr<storage::AbstractTable> loadWithStringHeader(const std::string &datafilepath, const std::string &header);

/*! Load store with main and delta from separate files
  @param mainfilepath
  @param deltafilepath
*/
std::shared_ptr<storage::Store> loadMainDelta(const std::string &mainfilepath, const std::string &deltafilepath, params p = params());

/**
 * Load a table into a raw table container
 */
std::shared_ptr<storage::AbstractTable> loadRaw(const std::string &file);

} } } } // end namespace hyrise::io::Loader::shortcuts

