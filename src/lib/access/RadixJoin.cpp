// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/RadixJoin.h"

#include "access/UnionAll.h"
#include "access/Barrier.h"
#include "access/system/BasicParser.h"
#include "access/system/ResponseTask.h"
#include "access/system/QueryParser.h"
#include "access/radixjoin/Histogram.h"
#include "access/radixjoin/NestedLoopEquiJoin.h"
#include "access/radixjoin/PrefixSum.h"
#include "access/radixjoin/RadixCluster.h"
#include "helper/types.h"
#include "log4cxx/logger.h"
#include <math.h>

namespace hyrise {
namespace access {

namespace {
auto _ = QueryParser::registerPlanOperation<RadixJoin>("RadixJoin");
log4cxx::LoggerPtr _logger(log4cxx::Logger::getLogger("hyrise.access"));
}

// Used to cap the hash_par and join_par degree
const size_t RadixJoin::MaxParallelizationDegree;

void RadixJoin::executePlanOperation() {}

std::shared_ptr<PlanOperation> RadixJoin::parse(const Json::Value& data) {
  auto instance = BasicParser<RadixJoin>::parse(data);
  instance->setBits1(data["bits1"].asUInt());
  instance->setBits2(data["bits2"].asUInt());
  return instance;
}

const std::string RadixJoin::vname() { return "RadixJoin"; }

void RadixJoin::setBits1(const uint32_t b) { _bits1 = b; }

void RadixJoin::setBits2(const uint32_t b) { _bits2 = b; }

uint32_t RadixJoin::bits1() const { return _bits1; }

uint32_t RadixJoin::bits2() const { return _bits2; }

size_t RadixJoin::getTableSize(size_t dep_index) {
  const auto& dep = std::dynamic_pointer_cast<PlanOperation>(_dependencies[dep_index]);
  if (!dep) {
    throw std::runtime_error("RadixJoin is missing one dependency");
  }
  const auto& inputTable = dep->getResultTable();
  if (!inputTable) {
    return 0;
  }
  return inputTable->size();
}

size_t RadixJoin::getHashTableSize() { return getTableSize(1); }

size_t RadixJoin::getProbeTableSize() { return getTableSize(0); }

// FIXME merge logic with RadixJoinTransformation.
std::vector<taskscheduler::task_ptr_t> RadixJoin::applyDynamicParallelization(
    taskscheduler::DynamicCount dynamicCount) {

  std::vector<taskscheduler::task_ptr_t> tasks;

  std::vector<taskscheduler::task_ptr_t> successors;
  {
    std::lock_guard<decltype(_observerMutex)> lk(_observerMutex);
    // get successors of current task
    for (auto doneObserver : _doneObservers) {
      if (auto sharedDoneObserver = doneObserver.lock()) {
        auto const task = std::dynamic_pointer_cast<taskscheduler::Task>(sharedDoneObserver);
        successors.push_back(task);
      }
    }
    // remove done observers from current task
    _doneObservers.clear();
  }

  size_t hash_par = dynamicCount.hash_par;
  size_t probe_par = dynamicCount.probe_par;
  size_t join_par = dynamicCount.join_par;

  // the original radix join task is not executed
  // instead we create the following tasks for radix join

  std::string opIdBase = _operatorId;


  // create ops and edges for probe side
  auto probe_side = build_probe_side(
      _operatorId + "_probe", _indexed_field_definition[0], probe_par, _bits1, _bits2, _dependencies[0]);

  tasks.insert(tasks.end(), probe_side.begin(), probe_side.end());

  // create ops and edges for hash side
  auto hash_side =
      build_hash_side(_operatorId + "_hash", _indexed_field_definition[1], hash_par, _bits1, _bits2, _dependencies[1]);

  tasks.insert(tasks.end(), hash_side.begin(), hash_side.end());

  // We have as many partitions as we have bits in the first pass have
  size_t partitions = 1 << _bits1;

  // create join ops
  int first = 0, last = 0;
  std::string join_name;
  // indexes into hash_ops and probe_ops to connect edges to join ops
  int probe_barrier = probe_side.size() - 1;
  int probe_prefix = probe_side.size() - 2;
  int hash_barrier = hash_side.size() - 1;
  int hash_prefix = hash_side.size() - 2;

  // case 1: no parallel join -> we do not need a union and have to connect the output edges to join
  if (join_par == 1) {
    join_name = _operatorId + "_join";

    auto j = std::make_shared<NestedLoopEquiJoin>();
    copyTaskAttributesFromThis(j);
    j->setOperatorId(_operatorId + "_join");
    j->setBits1(_bits1);
    j->setBits2(_bits2);
    j->setPlanOperationName("NestedLoopEquiJoin");

    for (size_t i = 0; i < partitions; i++) {
      j->addPartition(i);
    }

    // set join as dependency to all successors
    for (auto successor : successors)
      successor->addDependency(j);

    // probe input table
    j->addDoneDependency(_dependencies[0]);
    // probe barrier
    j->addDependency(probe_side[probe_barrier]);
    // probe merge prefix sum
    j->addDependency(probe_side[probe_prefix]);
    // hash input table
    j->addDoneDependency(_dependencies[1]);
    // hash barrier
    j->addDependency(hash_side[hash_barrier]);
    // hash merge prefix sum
    j->addDependency(hash_side[hash_prefix]);

    // set the single NestedLoopEquiJoin j as a dependency to original successors
    for (auto successor : successors) {
      successor->changeDependency(std::dynamic_pointer_cast<taskscheduler::Task>(shared_from_this()), j);
    }

    tasks.push_back(j);

  } else {  // case 2: parallel join -> we do need a union and have to connect the output edges to union
    auto unionall = std::make_shared<UnionAll>();
    unionall->setOperatorId(opIdBase + "_union");
    unionall->setPlanOperationName("UnionAll");
    copyTaskAttributesFromThis(unionall);

    // set union as dependency to all successors
    for (auto successor : successors)
      successor->changeDependency(shared_from_this(), unionall);

    // calculate partitions that need to be worked by join
    // if join_par > partitions, set join_par to partitions
    if (join_par > partitions) {
      join_par = partitions;
    }

    for (size_t i = 0; i < join_par; i++) {
      std::ostringstream s;
      s << i;

      auto j = std::make_shared<NestedLoopEquiJoin>();
      copyTaskAttributesFromThis(j);
      j->setOperatorId(_operatorId + "_join_" + s.str());
      j->setBits1(_bits1);
      j->setBits2(_bits2);
      j->setPlanOperationName("NestedLoopEquiJoin");

      distributePartitions(partitions, join_par, i, first, last);

      for (int i = first; i <= last; i++) {
        j->addPartition(i);
      }

      // create edges
      // probe input table
      j->addDoneDependency(_dependencies[0]);
      // probe barrier
      j->addDependency(probe_side[probe_barrier]);
      // probe merge prefix sum
      j->addDependency(probe_side[probe_prefix]);
      // hash input table
      j->addDoneDependency(_dependencies[1]);
      // hash barrier
      j->addDependency(hash_side[hash_barrier]);
      // hash merge prefix sum
      j->addDependency(hash_side[hash_prefix]);
      // out union
      unionall->addDependency(j);
      tasks.push_back(j);
    }
    tasks.push_back(unionall);
  }

  // register tasks at response task
  if (auto responseTask = getResponseTask()) {
    for (auto task : tasks) {
      responseTask->registerPlanOperation(std::dynamic_pointer_cast<PlanOperation>(task));
    }
  }

  return tasks;
}

taskscheduler::DynamicCount RadixJoin::determineDynamicCount(size_t maxTaskRunTime) {
  // this can never be satisfied. Default to NO parallelization.
  if (maxTaskRunTime == 0) {
    return taskscheduler::DynamicCount{1, 1, 1, 1};
  }

  auto hashTableSize = getHashTableSize() / (double)100000;
  auto probeTableSize = getProbeTableSize() / (double)100000;

  auto hash_instances = (_cluster_a * hashTableSize) / (maxTaskRunTime - _cluster_b * hashTableSize - _cluster_c);
  size_t hash_par = std::max(1, static_cast<int>(round(hash_instances)));
  if (hash_par > MaxParallelizationDegree) {
    hash_par = MaxParallelizationDegree;
  }

  auto probe_instances = (_cluster_a * probeTableSize) / (maxTaskRunTime - _cluster_b * probeTableSize - _cluster_c);
  size_t probe_par = std::max(1, static_cast<int>(round(probe_instances)));
  if (probe_par > MaxParallelizationDegree) {
    probe_par = MaxParallelizationDegree;
  }

  // The overall work is based on probe and hash table sizes
  // The overhead per instances is based on just the probe table size
  auto join_instances =
      (probeTableSize * (_join_d + _join_a * hashTableSize)) / (maxTaskRunTime - _join_b * probeTableSize - _join_c);
  size_t join_par = std::max(1, static_cast<int>(round(join_instances)));

  return taskscheduler::DynamicCount{1, hash_par, probe_par, join_par};
}

void RadixJoin::copyTaskAttributesFromThis(std::shared_ptr<PlanOperation> to) {
  to->setPriority(_priority);
  to->setSessionId(_sessionId);
  to->setPlanId(_planId);
  to->setTXContext(_txContext);
  to->setId(_txContext.tid);
  to->setEvent(_papiEvent);
}

std::vector<taskscheduler::task_ptr_t> RadixJoin::build_probe_side(std::string prefix,
                                                                   field_t& field,
                                                                   uint probe_par,
                                                                   uint32_t bits1,
                                                                   uint32_t bits2,
                                                                   taskscheduler::task_ptr_t input) {
  std::vector<taskscheduler::task_ptr_t> probe_side;

  // First define the plan ops
  // add parallel ops

  std::vector<taskscheduler::task_ptr_t> histograms;
  std::vector<taskscheduler::task_ptr_t> prefixsums;
  std::vector<taskscheduler::task_ptr_t> radixclusters;

  for (int i = 0; i < (int)probe_par; i++) {

    std::ostringstream s;
    s << i;

    auto h = std::make_shared<Histogram>();
    copyTaskAttributesFromThis(h);
    h->setOperatorId(prefix + "_probe_histogram_" + s.str());
    h->setBits(bits1);
    h->setCount(probe_par);
    h->setPart(i);
    h->addField(field);
    h->setPlanOperationName("Histogram");
    histograms.push_back(h);
    probe_side.push_back(h);

    auto p = std::make_shared<PrefixSum>();
    copyTaskAttributesFromThis(p);
    p->setOperatorId(prefix + "_probe_prefixsum_" + s.str());
    p->setCount(probe_par);
    p->setPart(i);
    p->setPlanOperationName("PrefixSum");
    prefixsums.push_back(p);
    probe_side.push_back(p);

    auto r = std::make_shared<RadixCluster>();
    copyTaskAttributesFromThis(r);
    r->setOperatorId(prefix + "_probe_radixcluster_" + s.str());
    r->setBits(bits1);
    r->setCount(probe_par);
    r->setPart(i);
    r->addField(field);
    r->setPlanOperationName("RadixCluster");
    radixclusters.push_back(r);
    probe_side.push_back(r);
  }

  auto c = std::make_shared<CreateRadixTable>();
  copyTaskAttributesFromThis(c);
  c->setOperatorId(prefix + "_probe_createradixtable");
  probe_side.push_back(c);
  c->setPlanOperationName("CreateRadixTable");

  auto m = std::make_shared<MergePrefixSum>();
  copyTaskAttributesFromThis(m);
  m->setOperatorId(prefix + "_probe_mergeprefixsum");
  m->setPlanOperationName("MergePrefixSum");
  probe_side.push_back(m);

  auto b = std::make_shared<Barrier>();
  copyTaskAttributesFromThis(b);
  b->setOperatorId(prefix + "_probe_barrier");
  b->setPlanOperationName("Barrier");
  probe_side.push_back(b);
  b->addField(field);


  // Then define the edges

  // There is an edge from input to create cluster table
  c->addDoneDependency(input);

  for (int i = 0; i < (int)probe_par; i++) {

    // the input goes to all histograms
    histograms[i]->addDoneDependency(input);

    // All equal histograms go to all prefix sums
    for (int j = 0; j < (int)probe_par; j++) {
      prefixsums[j]->addDependency(histograms[i]);
    }

    // And from each input there is a link to radix clustering
    radixclusters[i]->addDoneDependency(input);

    // From create radix table to radix cluster
    radixclusters[i]->addDependency(c);

    // From each prefix sum there is a link to radix clustering
    radixclusters[i]->addDependency(prefixsums[i]);

    //  Merge all prefix sums
    m->addDependency(prefixsums[i]);
    b->addDependency(radixclusters[i]);
  }

  return probe_side;
}


std::vector<taskscheduler::task_ptr_t> RadixJoin::build_hash_side(std::string prefix,
                                                                  field_t& field,
                                                                  uint hash_par,
                                                                  uint32_t bits1,
                                                                  uint32_t bits2,
                                                                  taskscheduler::task_ptr_t input) {

  std::vector<taskscheduler::task_ptr_t> hash_side;

  // First define the plan ops
  // add parallel ops

  std::vector<taskscheduler::task_ptr_t> histograms_p1;
  std::vector<taskscheduler::task_ptr_t> prefixsums_p1;
  std::vector<taskscheduler::task_ptr_t> radixclusters_p1;
  std::vector<taskscheduler::task_ptr_t> histograms_p2;
  std::vector<taskscheduler::task_ptr_t> prefixsums_p2;
  std::vector<taskscheduler::task_ptr_t> radixclusters_p2;

  for (int i = 0; i < (int)hash_par; i++) {

    std::ostringstream s;
    s << i;

    auto h = std::make_shared<Histogram>();
    copyTaskAttributesFromThis(h);
    h->setOperatorId(prefix + "_hash_histogram_1_" + s.str());
    h->setBits(bits1);
    h->setCount(hash_par);
    h->setPart(i);
    h->setPlanOperationName("Histogram");
    h->addField(field);
    histograms_p1.push_back(h);
    hash_side.push_back(h);

    auto p = std::make_shared<PrefixSum>();
    copyTaskAttributesFromThis(p);
    p->setOperatorId(prefix + "_hash_prefixsum_1_" + s.str());
    p->setCount(hash_par);
    p->setPart(i);
    p->setPlanOperationName("PrefixSum");
    prefixsums_p1.push_back(p);
    hash_side.push_back(p);

    auto r = std::make_shared<RadixCluster>();
    copyTaskAttributesFromThis(r);
    r->setOperatorId(prefix + "_hash_radixcluster_1_" + s.str());
    r->setBits(bits1);
    r->setCount(hash_par);
    r->setPlanOperationName("RadixCluster");
    r->setPart(i);
    r->addField(field);
    radixclusters_p1.push_back(r);
    hash_side.push_back(r);

    auto h2 = std::make_shared<Histogram2ndPass>();
    copyTaskAttributesFromThis(h2);
    h2->setOperatorId(prefix + "_hash_histogram_2_" + s.str());
    h2->setBits(bits1);
    h2->setBits2(bits2, bits1);
    h2->setCount(hash_par);
    h2->setPart(i);
    h2->setPlanOperationName("Histogram2ndPass");
    histograms_p2.push_back(h2);
    hash_side.push_back(h2);

    auto p2 = std::make_shared<PrefixSum>();
    copyTaskAttributesFromThis(p2);
    p2->setOperatorId(prefix + "_hash_prefixsum_2_" + s.str());
    p2->setCount(hash_par);
    p2->setPart(i);
    p2->setPlanOperationName("PrefixSum");
    prefixsums_p2.push_back(p2);
    hash_side.push_back(p2);

    auto r2 = std::make_shared<RadixCluster2ndPass>();
    copyTaskAttributesFromThis(r2);
    r2->setOperatorId(prefix + "_hash_radixcluster_2_" + s.str());
    r2->setBits1(bits1);
    r2->setBits2(bits2, bits1);
    r2->setCount(hash_par);
    r2->setPart(i);
    r2->setPlanOperationName("Histogram");
    radixclusters_p2.push_back(r2);
    hash_side.push_back(r2);
  }

  auto c = std::make_shared<CreateRadixTable>();
  copyTaskAttributesFromThis(c);
  c->setOperatorId(prefix + "_hash_createradixtable_1");
  hash_side.push_back(c);
  c->setPlanOperationName("CreateRadixTable");

  auto c2 = std::make_shared<CreateRadixTable>();
  copyTaskAttributesFromThis(c2);
  c2->setOperatorId(prefix + "_hash_createradixtable_2");
  hash_side.push_back(c2);
  c2->setPlanOperationName("CreateRadixTable");

  auto m = std::make_shared<MergePrefixSum>();
  copyTaskAttributesFromThis(m);
  m->setOperatorId(prefix + "_hash_mergeprefixsum");
  m->setPlanOperationName("MergePrefixSum");
  hash_side.push_back(m);

  auto b = std::make_shared<Barrier>();
  copyTaskAttributesFromThis(b);
  b->setPlanOperationName("Barrier");
  b->setOperatorId(prefix + "_hash_barrier");
  hash_side.push_back(b);
  b->addField(field);

  // Then define the edges
  // There is an edge from input to create cluster table and for the second pass
  c->addDoneDependency(input);
  c2->addDoneDependency(input);

  for (size_t i = 0; i < hash_par; i++) {

    // the input goes to all histograms
    histograms_p1[i]->addDoneDependency(input);

    // All equal histograms go to all prefix sums
    for (int j = 0; j < (int)hash_par; j++) {
      prefixsums_p1[j]->addDependency(histograms_p1[i]);
    }

    // And from each input there is a link to radix clustering
    radixclusters_p1[i]->addDoneDependency(input);

    // From create radix table to radix cluster
    radixclusters_p1[i]->addDependency(c);

    // From each prefix sum there is a link to radix clustering
    radixclusters_p1[i]->addDependency(prefixsums_p1[i]);

    // now comes the second pass which is like the first only a litte
    // more complicated
    for (int j = 0; j < (int)hash_par; j++) {
      // We need an explicit barrier here to avoid that a histogram is calculated before all other
      // first pass radix clusters finished
      histograms_p2[j]->addDependency(radixclusters_p1[i]);
      prefixsums_p2[i]->addDependency(histograms_p2[j]);
    }
    // This builds up the second pass radix cluster, attention order matters
    radixclusters_p2[i]->addDependency(radixclusters_p1[i]);
    radixclusters_p2[i]->addDependency(c2);
    radixclusters_p2[i]->addDependency(prefixsums_p2[i]);
    m->addDependency(prefixsums_p2[i]);
    b->addDependency(radixclusters_p2[i]);
  }
  return hash_side;
}

void RadixJoin::distributePartitions(const int partitions,
                                     const int join_count,
                                     const int current_join,
                                     int& first,
                                     int& last) const {
  const int partitionsPerJoin = partitions / join_count,
            remainingElements = partitions - partitionsPerJoin * join_count,
            extraElements = current_join <= remainingElements ? current_join : remainingElements,
            partsExtraElement = current_join < remainingElements ? 1 : 0;
  first = partitionsPerJoin * current_join + extraElements;
  last = first + partitionsPerJoin + partsExtraElement - 1;
}
}
}
