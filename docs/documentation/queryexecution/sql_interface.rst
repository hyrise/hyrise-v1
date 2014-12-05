#######################
SQL Interface
#######################

Hyrise has an experimental SQL interface that will execute SQL queries based on the available operators. On this page you will find information about the current state of this interface. Usually we will keep track here what has yet to be implemented or where there are problems.
For updates on the SQL-Parser look at https://github.com/hyrise/sql-parser.


Missing Features
----------------

This is a list of features that haven't been implemented yet. Usually the SQL Parser can handle all of these already, but there is no appropriate mapping to Hyrise operators yet.

  * Order By

  * Distinct select

    * SELECT DISTINCT grade FROM students
    * SELECT COUNT(DISTINCT grade) FROM students GROUP BY name;

  * Merge

    * MERGE TABLE students

  * Having clause

    * Unknown if implemented in Hyrise

  * Creating indices

  * DROP table (TableUnload)

  * Creating hybrid tables (mixed row/column layout)

  * Update

    * There is a UpdateScan operator but there is no example for UpdateScan in test/.

  * Delete/Truncate
    
    * No example for Delete PlanOp in test/. Might work through generating positions through SimpleTableScan and then calling Delete.

  * Union (see below)

    * UnionScan only supports the same table as inputs and needs positions to be created beforehand.
