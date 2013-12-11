// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#ifdef WITH_MYSQL

#include <stdlib.h>

#include "helper/Environment.h"
#include "io/AbstractLoader.h"

namespace hyrise {
namespace io {

const std::string ENV_MYSQL_HOST = "HYRISE_MYSQL_HOST";
const std::string ENV_MYSQL_PORT = "HYRISE_MYSQL_PORT";
const std::string ENV_MYSQL_USER = "HYRISE_MYSQL_USER";
const std::string ENV_MYSQL_PASS = "HYRISE_MYSQL_PASS";

class MySQLInput : public AbstractInput {
public:
  class params {
#include "parameters.inc"
  public:
    param_member(std::string, Host);
    param_member(std::string, Port);
    param_member(std::string, User);
    param_member(std::string, Password);
    param_member(std::string, Schema);
    param_member(std::string, Table);
    param_member(uint64_t, Limit);
    params() : Host(getEnv(ENV_MYSQL_HOST, "127.0.0.1")),
      Port(getEnv(ENV_MYSQL_PORT, "3306")),
      User(getEnv(ENV_MYSQL_USER, "root")),
      Password(getEnv(ENV_MYSQL_PASS, "root")),
      Limit(0)
    {}
  };

  MySQLInput(const params &parameters = params()) :
    _parameters(parameters)
  {}

  std::shared_ptr<storage::AbstractTable> load(std::shared_ptr<storage::AbstractTable>,
                                               const storage::compound_metadata_list *,
                                               const Loader::params &args);

  MySQLInput *clone() const;

private:
  params _parameters;
};

} } // namespace hyrise::io

#endif

