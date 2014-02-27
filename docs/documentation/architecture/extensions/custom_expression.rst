##################
Custom expressions
##################


Why write a custom expression?
==============================

Writing custom expressions allows implementing an expression evaluation
type that is not yet (or will ever be) supported. Only a vector of
matching row numbers needs to be returned, thus allowing for
relatively simple custom implementations. Custom expression may also
collapse complex expression trees into closed functions, allowing for
higher performance in evaluating complex expressions where needed.

For more complex needs, see :doc:`../../architecture/extensions/custom_operator`.

`TableScan` implements a registration-based expression evaluation for
custom implementations of expressions. The idea for custom expressions
is that they follow the expression protocol set forth in
`AbstractExpression` and register via
`Expressions::add<...>("callsign")`. An example can be found in
`src/lib/access/ExampleExpression.(h|cpp)`.


`AbstractExpression` interface
==============================


`walk` method
-------------

This method is called once the input table has been determined in
`TableScan::setupPlanOperation`. The input table is then passed a 
shared pointer to an `AbstractTable`. At this point, we can take full
control and start to recover type information and "peel off" layers of
abstraction in order to allow for more efficient expression
processing.

After `walk` is done, you should have recovered all the necessary data
structures to process your query: The already mentioned
`ExampleExpression` extracts the underlying attribute vectors and
dictionaries and ensures the correct type of the retrieved data
structures.


`match` method
--------------

Then, in a second step, the actual scan is executed via calling `match` on
the operator. The interface basically assumes that `match` returns a
list of positions that can be consumed by a `PointerCalculator`.
In its simplest form, the match method will just iterate over its
given inputs and add a row number for each matching rows. More complex
match methods may spawn extra threads or use different schemes of
iterating over extracted data structures from the `walk` method. 


Registration and usage
======================


Registering
-----------

A new expression can be added by placing the following code into an
implementation file::

  namespace {
    auto _ = Expressions::add<ExampleExpression>("hyrise::example");
  }

`hyrise::example` is a unique expression identifier that we may later
use to refer to `ExampleExpression`. When implementing your
expression, we suggest to use sufficiently unique identifiers to avoid
future colisions with either hyrise-implemented expressions or other
extensions by namespacing your expressions similarly to the above example.


Parsing
-------

Since custom expressions may need custom parameters and very different
handling from HYRISE's built-in expressions, they have to implement a
custom static `parse` method, which allows them to arbitrarily extend the
JSON syntax of the `TableScan` operator with their own fields.

The minimum implementations may look something like::

  std::unique_ptr<MyExpression> ExampleExpression::parse(const Json::Value& data) {
    return make_unique<ExpressionExpression>();
  }


Using registered expressions
----------------------------

Once registered, an expression may be used through the `TableScan`
operator in JSON plans through using the `"expression":` parameter::

  { 
  "operators": {
    "scan" : {
      "type": "TableScan",
      "expression": "hyrise::example"
      /*...*/
    }
  }

See an example for the usage of `hyrise::example` in `test/autojson/example_expression.json`.
