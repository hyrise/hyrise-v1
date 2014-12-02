#######################
SQL Interface
#######################

Hyrise has an experimental SQL interface that will execute SQL queries based on the available operators. On this page you will find information about the current state of this interface. Usually we will keep track here what has yet to be implemented or where there are problems.


Missing Features
----------------

This is a list of features that haven't been implemented yet. Usually the SQL Parser can handle all of these already, but there is no appropriate mapping to Hyrise operators yet.

  * Order By
  * Distinct select
  * Merge
  * Having clause
  * Creating indices
  * Delete table (TableUnload)
  * Creating hybrid tables
  * Update (see below)
  * Delete/Truncate (see below)
  * Union (see below)



Current problems
----------------

This is a short overview over known problems, that need to be resolved to implement more features.


Delete/Truncate statements
**************************

No example for Delete PlanOp in test/. Might work through generating positions through SimpleTableScan and then calling Delete.


Update Statement
****************

There is a UpdateScan operator but there is no example for UpdateScan in test/.


Union
*****

UnionScan only supports the same table as inputs and needs positions to be created beforehand.


Unknown/Unimplemented in Hyrise
*******************************

Features that Hyrise can't handle at the moment (or it is not known how):

  * Having clauses