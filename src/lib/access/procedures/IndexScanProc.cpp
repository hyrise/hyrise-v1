#include "IndexScanProc.h"

#include "access/IndexScan.h"
#include "access/tx/Commit.h"
#include "access/storage/GetTable.h"
#include "access/system/ResponseTask.h"

#include "io/TransactionManager.h"

#include "taskscheduler/SharedScheduler.h"

#include "helper/HttpHelper.h"



namespace hyrise { namespace access {

bool IndexScanProcedure::registered = net::Router::registerRoute<IndexScanProcedure>("/proc/indexscan");


IndexScanProcedure::IndexScanProcedure(net::AbstractConnection *data) : _connection_data(data) {
}


void IndexScanProcedure::operator()() {
  // Parse the query string
  std::map<std::string, std::string> body_data = parseHTTPFormData(_connection_data->getBody());

  auto gt = std::make_shared<access::GetTable>(body_data["table"]);
  
  auto id = std::make_shared<access::IndexScan>();
  id->setIndexName(body_data["index"]);
  id->addField(0);
  id->setPriority(1);
  id->setValue<hyrise_int_t>(atol(body_data["value"].c_str()));

  auto ctx= tx::TransactionManager::beginTransaction();
  auto ci = std::make_shared<access::Commit>();
  ci->setTXContext(ctx);

  auto rt = std::make_shared<access::ResponseTask>(_connection_data);
  rt->setTxContext(ctx);
  rt->setRecordPerformanceData(false);
  rt->setPriority(1);

  rt->registerPlanOperation(gt);
  rt->registerPlanOperation(id);
  rt->registerPlanOperation(ci);

  rt->addDependency(ci);
  ci->addDependency(id);
  id->addDependency(gt);

  if (atoi(body_data["limit"].c_str()) > 0)
    rt->setTransmitLimit(atol(body_data["limit"].c_str()));
  
  if (atoi(body_data["offset"].c_str()) > 0)
    rt->setTransmitOffset(atol(body_data["offset"].c_str()));

  (*gt)();
  gt->notifyDoneObservers();

  (*id)();
  id->notifyDoneObservers();

  (*ci)();
  ci->notifyDoneObservers();

  (*rt)();
  rt.reset();
}
}}
