#ifndef SRC_LIB_ACCESS_REQUESTPARSETASK_H_
#define SRC_LIB_ACCESS_REQUESTPARSETASK_H_

#include <string>
#include <memory>

#include "net/Router.h"
#include "net/AbstractConnection.h"

namespace hyrise {
namespace access {

class ResponseTask;

class RequestParseTask : public net::AbstractRequestHandler {
 private:
  net::AbstractConnection *_connection;
  std::shared_ptr<ResponseTask> _responseTask;
 public:
  explicit RequestParseTask(net::AbstractConnection *connection);
  virtual ~RequestParseTask();
  std::shared_ptr<ResponseTask> getResponseTask() const;
  virtual void operator()();
  static std::string name();
  const std::string vname();
};

}
}

#endif  // SRC_LIB_ACCESS_REQUESTPARSETASK_H_
