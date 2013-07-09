******************************
Transaction Management System
******************************

HYRISE uses optimistic concurrency control mechanisms to provide transactional
guarantees during query execution. However, currently HYRISE is only able to
guarantee atomicity, consistency, and isolation but not durability since
currently no logging is used to store the transactions on disk.

In addition, transactions in HYRISE are not enabled by default, but are rather can be additionally selected when building the query plan. 


Architectural Overview
=======================

The ``TransactionManager`` class in HYRISE is the central point of
serialization for transactions. During query parsing each query is assigned a
``TXContext`` that can be used to identify all matching positions. It contains
a unique transaction ID and the last commit id. The transaction ID is used to
distinguish other deletes and writes from our own deletes and writes.

Typical Control Flow
=====================

Typically, we can distinguish between two different scenarios: Read-Only
Access and Read-Write Access.

For read-only data access the control flow is hereby as follows:

  1. Perform Scan on the physical table and it is required to use position lists (aka PointerCalculator) only
  2. On each intermediate result from a physical table execute the plan
     operation ``ValidatePositions``. This PO will check given the current
     transaction context if the records satisfy the current state of the
     transaction and are visible. Once this operation is done, we can assume
     for the rest of the query that all records we read using these position
     lists are valid

For read-write access the control flow is similar:

  1. Perform a scan on the physical tables
  2. Apply the plan operation ``ValidatePositions`` to remove invisible records
  3. Call ``Commit`` plan operation in the end

Currently a single commit plan operation will guarantee the correct execution
of the transaction for all its input tables. However, TX are then serialized by commits in the TX manager.

Once a transaction is commited, the TX context is no longer valid and the
system is required to fetch a new TID to proceed.

Necessary Plan Operations
==========================

The following plan operations are tightly integrated with the TX management
and should only be used in this context:

  1. ``ValidatePositions`` - validates the input PointerCalculator so that only
     valid positions remain
  2. ``Commit`` - Commits the current transaction on this table, updates the
     commit ID for all modified records when done
  3. ``InsertScan`` - Inserts new records into the delta. The new state of
     inserted records is that they are invalid and the TID column of the record is set to the TID of the transaction to identify own writes
  4. ``PosUpdateScan`` - performs an update on an input table. The
     modifications are specified in the JSON of the plan operation. An update
     is basically a delete + insert of the modified tuple



