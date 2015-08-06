################
Query Statistics
################

The goal of query statistics is to keep track of what is happening in
the system. To achieve this, information about the execution of each
plan operation is stored in a special statistics table.

To avoid contention when writing to the plan statistics table each
plan operation primarily operates on a special thread local
storage. Since only one plan operation can be executed by a single
thread at a time no additional locks are needed.

From time to time this information is merged into the central
statistics table, but it is not guaranteed that each query will
appear. We assume that if a single query might not be recorded this
information will have no impact on the overall workload.


Statistic Table
===============

The table for the query statistics has the following columns ---
independent if it is the global or thread local table.

===========  =====================================================
Name         Description
===========  =====================================================
plan_id      The sha1 hash of the plan for further identification
operator     The vname() of the operator
detail       n/a
selectivity  The selectivity of the operator
start        The start timestamp of the plan operator
===========  =====================================================


