******************************
Transaction Management System
******************************

HYRISE uses optimistic concurrency control mechanisms to provide transactional
guarantees during query execution. However, currently HYRISE is only able to
guarantee atomicity, consistency, and isolation but not durability since
currently no logging is used to store the transactions on disk.


Using transactions in your plan
===============================

Transactions work on the basis of session contexts. Every executed query plan
receives a transaction context (`TXContext`) as a part of parsing the plan. This
context allows plan operations to act based on the context.

There are two essential parts to transaction control in HYRISE:

1. Creating a transaction context/reusing an existing transaction context
2. Commit/Rollback transaction context

Single-request transactions
---------------------------

Typically, we will run plans which are self-contained, as an example
we may grab a table and insert a few records and then commit those
records in order to make our changes visible to other transactions.

.. literalinclude:: ../../test/autojson/insert_commit.json
   :language: javascript
   :linenos:
   :lines: 1-2,8-25,27-
   :emphasize-lines: 4-5,8-13,17

The above example shows what a typical insert looks like: In line
`retrieve_revenue`, we retrieve a handle to the `revenue` table, which
has 3 columns (year, month, revenue).

Then, we continue by inserting 4 new rows into `revenue`. Finally, we
expose the inserted rows to other transactions by committing them in
the `commit` operation.

Let's use this example to examine the essential parts of transaction
control: Context creation and committing of the same.

.. seqdiag::

   seqdiag {
      autonumber = True;
      activation = none;

      client -> server [label = "POST /query/"];
      server -> parser [label = "query plan"];
      parser => txmanager [label = "create new context", return="context"]
      parser --> server [label = "plan operations"];
      === execute plan operations ===
      server => txmanager [label = "commit context", return="commit_id"]
      client <-- server [label = "response"];
   }

Context creation happens during the initial plan parsing: We create a
new plan context during every request (if you need different behavior,
look at the :ref:`next section <multiple-tx>`). All the operations
created for our request may then use the context to carry out their
operations. As an example ``InsertScan`` uses the context to insert rows
marked with our *unique* transaction so other transactions will not
assume that they are already visible.

Finally, we commit the automatically created context in
``Commit``. Commit basically checks for possible conflicts with other
transactions - in the case of plain insertion of rows, such a conflict
is not possible and thus, our example should always succeed.

.. important:

   Once the execution of `insert` finishes, these 4 records are then
   visible to all operations within the same context, but not for
   other transactions. Only `commit` makes them available to other
   transactions.


.. _multiple-tx:

Multiple transaction steps
--------------------------

While the behavior in the beforehand example is sufficient for simple
transactions, more elaborate transactions involving multiple queries
and logic carried out on the client naturally involve transaction
contexts that live longer than one request.

.. literalinclude:: ../../test/autojson/insert_wo_commit.json
   :lines: 1-2,8-22,24-
   :language: javascript
   :linenos:

Now in this query, we don't use the ``Commit`` -Operation. Instead, we
leave the context in an uncommitted state.

The server will the send a response similar to the following::

    {
       "session_context" : "somecontext",
       "performanceData" : { /*...*/ },
       /* and more ... */
    }

Since the session was not ended, it is up to the client to re-use the
``session_context`` returned by the query to eventually use the
``Commit`` or ``Rollback`` plan operation to end the session.

.. note:

   The composition of `session_context` may change and should not be altered
   by the client. It is possible to send multiple queries concurrently with
   the same `session_context`.

.. note:

   When a session did not alter any data, there is no need to commit
   or rollback the session explicitly.

The following sequence diagram illustrates the first request.

.. seqdiag::

   seqdiag {
      autonumber = True;
      activation = none;

      client -> server [label = "POST /query/"];
      server -> parser [label = "query plan"];
      parser => txmanager [label = "create new context", return="context"]
      parser --> server [label = "plan operations"];
      === execute plan operations `retrieve_revenue` and `insert` ===
      client <-- server [label = "response with context"];
   }

To reuse an existing session context, we need to add the
``session_context`` as an additional ``POST`` parameter our HTTP
request to `/query/`. An example implementation for Python can be
found in ``tools/client.py``.

Following requests using a ``session_context`` post parameter will not
create a new session context but re-use the existing context. If we
would rerun the above query with a set session context, the sequence
of activities would change accordingly:

.. seqdiag::

   seqdiag {
      autonumber = True;
      activation = none;

      client -> server [label = "POST /query/ with session context"];
      server -> parser [label = "query plan"];
      parser => txmanager [label = "retrieve from session context", return="context"]
      parser --> server [label = "plan operations"];
      === execute plan operations `retrieve_revenue` and `insert` ===
      client <-- server [label = "response with context"];
   }

Eventually, we will want to end the transaction and make our changes
visible to other transactions. This can be done by either extending
the last query with a ``Commit`` operation or a plan simply consisting
of one single ``Commit`` operation::

  { "operations": {"commit_op": {"type" : "Commit"} } }

This would then result in a response without ``session_context`` as
the session has been closed at this point.

.. seqdiag ::

   seqdiag {
      autonumber = True;
      activation = none;

      client -> server [label = "POST /query/ with session context"];
      server -> parser [label = "query plan"];
      parser => txmanager [label = "retrieve from session context", return="context"]
      parser --> server [label = "plan operations"];
      server => txmanager [label = "commit context", return="commit_id"]
      client <-- server [label = "response"];
   }

.. important:

   Reusing a session_context that has been committed/rolled back
   results in *undefined behavior*.

Autcommit shortcut
------------------

As explicitly writing out commit operations at the end of your queries
is tedious and error-prone, the ``/query/`` interface accepts an
additional ``POST`` parameter: ``autocommit``. This allows the query
parser to automatically append a ``Commit`` operation at the end of
the current query.

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

Once a transaction is committed, the TX context is no longer valid and the
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
     is basically a delete + insert of the modified tuple\

Multi Version Concurrency Control (MVCC)
=========================================

We use MVCC to isolate transactions. This section explains MVCC in general; see the next section for details specific to our implementation.

MVCC is a method to allow multiple transactions to write on the same table concurrently while still adhering to the ACID criteria. This means that every transaction should get the impression that it is the only thing working on the table (isolation). Changes made by other transactions shall only be visible if they have been committed before a transaction was started. Changes that were done in between are not visible. Every transaction has a consistent view of the table. If a transaction commits, its changes will be made visible to new transactions all-or-nothing (atomicity). Changes made by one transaction will not get overwritten by other transactions (durability).

This is done by storing multiple versions of the same row. Instead of updating a row, it is deleted (more correctly, made invisible for new transactions) and inserted with modified data (only visible to new transactions). This means that transactions that were started before the modifying transactions was committed can still access the version that was valid when they started.

Our MVCC Implementation
========================


On the level of a Store we do so by having three vectors storing visibility information about each row:

  * TID stores the TID of the transaction that is currently modifying the row
  * BeginCID stores the commit id of the committed transaction that inserted the row
  * EndCID stores the commit id of the committed transaction that deleted the row

The TID vector is used by a transaction to mark that the row is being modified by the transaction identified by TID x. Obviously, the vector can only store one TID per row - thus, every row can only be modified by one transaction.

If a row is inserted (but not committed), it has the TID of the transaction inserting it, but no begin and end commit id (see below). At this point, it is only visible if its TID is equal to the TID of a read operation. If a row is deleted (but the delete is uncommitted), it gets the TID of the deleting transaction. It is now invisible to this transaction but visible to others.

During the commit of an insert, the transaction stores its commit id (acquired from the TransactionManager) in the beginCID vector. As soon as the last commit id is increased in the TransactionManager (meaning that lastCID >= beginCID), the row is visible to new transactions.

The endCID vector behaves similar. It stores the CID of the transaction that successfully deleted the row and was committed.

+-------+------------+----------+-----------+-------------------------------+
| v.TID | v.BeginCID | v.EndCID | LastCID   | Status                        |
+=======+============+==========+===========+===============================+
| --    | Inf        | Inf      | 12        | Init                          |
+-------+------------+----------+-----------+-------------------------------+
| 5     | Inf        | Inf      | 12        | Insert uncommitted            |
+-------+------------+----------+-----------+-------------------------------+
| --    | 13         | Inf      | 12        | Insert commit in prog.        |
+-------+------------+----------+-----------+-------------------------------+
| --    | 13         | Inf      | 13        | Insert committed              |
+-------+------------+----------+-----------+-------------------------------+
| 6     | 13         | Inf      | 13        | delete uncommitted            |
+-------+------------+----------+-----------+-------------------------------+
| --    | 13         | 14       | 13        | delete commit in prog.        |
+-------+------------+----------+-----------+-------------------------------+
| --    | 13         | 14       | 14        | delete committed              |
+-------+------------+----------+-----------+-------------------------------+

Evaluating the visiblity
=========================

When checking if a row is visible to a transaction, three checks have to be made. These are shown in the table below.

+-----------------+--------------------------+------------------------+----------+---------------------------------------+
| v.TID == tx.TID | tx.LastCID >= v.BeginCID | tx.LastCID >= v.EndCID | Visible? | Comment                               |
+=================+==========================+========================+==========+=======================================+
| yes             | yes                      | yes                    | No       | Impossible                            |
+-----------------+--------------------------+------------------------+----------+---------------------------------------+
| no              | yes                      | yes                    | No       | Past Delete                           |
+-----------------+--------------------------+------------------------+----------+---------------------------------------+
| yes             | no                       | yes                    | No       | Impossible                            |
+-----------------+--------------------------+------------------------+----------+---------------------------------------+
| yes             | yes                      | no                     | No       | Own Delete, uncommitted               |
+-----------------+--------------------------+------------------------+----------+---------------------------------------+
| no              | no                       | yes                    | No       | Impossible                            |
+-----------------+--------------------------+------------------------+----------+---------------------------------------+
| yes             | no                       | no                     | Yes      | Own Insert                            |
+-----------------+--------------------------+------------------------+----------+---------------------------------------+
| no              | yes                      | no                     | Yes      | Past Insert or Future Delete          |
+-----------------+--------------------------+------------------------+----------+---------------------------------------+
| no              | no                       | no                     | No       | Uncommitted Insert or Future Insert   |
+-----------------+--------------------------+------------------------+----------+---------------------------------------+
