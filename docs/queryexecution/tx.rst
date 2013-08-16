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
       "session_context" : 12355,
       "performanceData" : { /*...*/ },
       /* and more ... */
    }

Since the session was not ended, it is up to the client to re-use the
``session_context`` returned by the query to eventually use the
``Commit`` or ``Rollback`` plan operation to end the session.

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

Possible TID Combinations
==========================

::

   +----------------+---------------+--------+--------+---------------+
   | CID > lastCID  | TID = tx.TID  | valid  | Keep?  | Comment       |
   +================+===============+========+========+===============+
   | yes            |  yes          | yes    | --     | impossible    |
   | no             |  yes          | yes    | --     | impossible    |
   | yes            |  no           | yes    | --     | Future insert |
   | yes            |  yes          | no     | --     | impossible    |
   | no             |  no           | yes    | --     | Past insert   |
   | yes            |  no           | no     | --     | Future delete |
   | no             |  yes          | no     | --     | Own write     |
   | no             |  no           | no     | --     | Past delete   |
   +----------------+---------------+--------+--------+---------------+
