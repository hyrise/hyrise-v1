// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <stddef.h>
#include <string>
#include <vector>
#include <stdexcept>

#include "helper/types.h"

#include "AbstractTable.h"

namespace hyrise { namespace storage {

class TableBuilderError : public std::runtime_error {
public:

  explicit TableBuilderError(const std::string &what): std::runtime_error(what)
  {}
};

#define builder_param_member(type, name) type name; param &set_##name(type t) { name = t; return *this; }

/*
  The goal of the table builder is to easily create a new modifiable
  table using a simple builder pattern.
*/
class TableBuilder {

public:

  struct param {
    builder_param_member(std::string, name);
    builder_param_member(std::string, type);
    param(): name(), type() {};
    param(std::string n, std::string t) : name(n), type(t) {}
  };


  /*
    A param list contains a list of parameters that can be simply
    appended by calling the append method. Append returns a
    reference to the newly added element and can be further chained
    to set the value of param_t
  */
  struct param_list {
    typedef TableBuilder::param param_t;
    typedef std::vector<param_t> param_list_t;
    typedef std::vector<unsigned> group_list_t;

    // layout for table
    group_list_t _groups;


    // internal param storage
    param_list_t _params;


    /*
      Taks a param_t as input and returns a referene to the
      element in the list for further chaining
    */
    param_t &append(param_t newElement = param_t()) {
      _params.push_back(newElement);
      return _params.back();
    }

    param_list &appendGroup(unsigned g) {
      _groups.push_back(g);
      return *this;
    }

    /*
     * Returns a const reference to the list of parameters
     */
    const param_list_t &params() const {
      return _params;
    }

    const group_list_t &groups() const {
      return _groups;
    }

    size_t size() const {
      return _params.size();
    }

    const param_t &operator[](const size_t index) {
      return _params[index];
    }

    param_list(): _groups(), _params()
    {}
  };

  /*
   */
  static atable_ptr_t build(param_list args, const bool compressed = false);


private:

  /*
    Based on a give iterator over a sequence of param_t this method
  */
  static atable_ptr_t createTable(param_list::param_list_t::const_iterator b,
      param_list::param_list_t::const_iterator e,
      const bool compressed);


  /*
    Performs a validity check on the input parameters. It can only
    detect those errors, that are based on static assumptions. This
    includes if the layout matches the numbers of columns etc.
  */
  static void checkParams(const param_list &args);

};

}}

