#include "io/TransactionRouteHandler.h"

#include <sstream>

#include "io/TransactionManager.h"
#include "helper/HttpHelper.h"
#include "helper/numerical_converter.h"

#include "cereal/archives/json.hpp"
#include "cereal/types/vector.hpp"

namespace hyrise { namespace tx {

namespace {
auto reg_status = net::Router::registerRoute<TransactionStatusHandler>("/status/tx");
}

void RequestHandler::operator()() {
  try {
    _connection->respond(processRequest());
  } catch (std::exception& e) {
    _connection->respond(std::string("error: ") + e.what(), 500);
  }
}

std::string TransactionStatusHandler::processRequest() {
  std::ostringstream out;
  {
    cereal::JSONOutputArchive archive(out);
    archive(cereal::make_nvp("contexts", TransactionManager::getRunningTransactionContexts()));
  }
  return out.str();
}

}}
