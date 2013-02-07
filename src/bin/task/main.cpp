/*#include <io.h>
  #include <access.h>
  #include <helper.h>
  #include <helper/PapiTracer.h>
  #include <storage.h>

  #include "taskscheduler.h"

  #define POOL_ID 0*/

//TODO(jwust): Is this still needed?

int main(int argc, char *argv[]) {
  /*

    Store *store = MergeLoader::load("test/lin_xxxs2.tbl");
    AbstractTable *t = store->getMainTables()[0];
    t->print(10);

    std::vector<TableOutputTask*> tasks;

    TableOutputTask *t1 = new TableOutputTask(t);
    tasks.push_back(t1);

    ProjectionScan *p = new ProjectionScan();
    p->addField(0);
    p->addDependency(t1);
    tasks.push_back(p);

    boost::threadpool::pool_manager *manager = boost::threadpool::pool_manager::sharedManager();
    // manager->resizePool(2, POOL_ID);

    std::vector<TableOutputTask*>::iterator i;
    for (i = tasks.begin(); i != tasks.end(); i++) {
    manager->schedule(*i, POOL_ID);
    }

    manager->getPool(POOL_ID)->wait();

    p->getOutput()->print();

    return 0;


    Store *store = MergeLoader::load("test/lin_xxxs2.tbl");
    AbstractTable *t = store->getMainTables()[0];
    t->print(10);

    std::vector<TableOutputTask*> tasks;

    TableOutputTask *t1 = new TableOutputTask(t);
    tasks.push_back(t1);

    ProjectionScan *p = new ProjectionScan();
    p->addField(0);
    p->addDependency(t1);
    tasks.push_back(p);

    boost::threadpool::pool_manager *manager = boost::threadpool::pool_manager::sharedManager();
    // manager->resizePool(2, POOL_ID);

    std::vector<TableOutputTask*>::iterator i;
    for (i = tasks.begin(); i != tasks.end(); i++) {
    manager->schedule(*i, POOL_ID);
    }

    manager->getPool(POOL_ID)->wait();

    p->getOutput()->print();

    return 0;
  */
}
