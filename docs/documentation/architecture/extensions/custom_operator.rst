#########################
Writing a custom operator
#########################

HYRISE allows for the implementation and registration of custom
operators to fit specific user needs that are not met by the built-in
operators. 

We currently distinguish two kinds of operators: `trivial` and
`complex` ones. A trivial operator does not need any special
construction from the JSON plan whereas a complex operator needs to do
custom parsing of JSON parameters. Trivial operators take inputs and
do something with them without any further parameterization.


Writing a `trivial` operator
============================

A trivial operator looks as follows in an execution plan::

  { "type" : "MyTrivialOperator" }

An implementation of `MyTrivialOperator` may look as follows.

::

   // MyTrivialOperator.h

   #ifndef MYTRIVIALOPERATOR_H
   #define MYTRIVIALOPERATOR_H

   #include "access/PlanOperation.h"

   class MyTrivialOperator : public _PlanOperation { 
      void executePlanOperation();
   }

   #endif

   // MyTrivialOperator.cpp

   #include "MyTrivialOperator.h"

   namespace { const auto _ =
     QueryParser::registerTrivialPlanOperation<MyTrivialOperator>("MyTrivialOperator");
   }

   void MyTrivialOperator::executePlanOperation() {
     // work your magic
   }

An example for such an operator could be an operator that takes all
its inputs and concatenates them. Such an operation needs no special
parameters.

In order to be able to register a trivial operation, the class must be
default-constructable.


Writing a `complex` operator
============================

In contrast to a `trivial` operator, a  `complex` operator may define
its own parsing routine, so it can access additional parameters
defined in the JSON execution plan::

  { "type" : "MyComplexOperator",
    "param" : "test" }

Such a complex operator may be constructed as follows::

   // MyComplexOperator.h

   #ifndef MYCOMPLEXOPERATOR_H
   #define MYCOMPLEXOPERATOR_H

   #include "access/PlanOperation.h"

   class MyComplexOperator : public _PlanOperation { 
      std::string _param;
      void executePlanOperation();
   public:
      MyComplexOperator(std::string param);
      static std::shared_ptr<_PlanOperation> parse(Json::Value& data);
   }

   #endif

   // MyComplexOperator.cpp

   #include "MyComplexOperator.h"

   namespace { const auto _ =
     QueryParser::registerPlanOperation<MyComplexOperator>("MyComplexOperator");
   }

   MyComplexOperator::MyComplexOperator(std::string param) :
      _param(param) {}
   
   void MyComplexOperator::executePlanOperation() {
     // work your magic
   }
   
   std::shared_ptr<_PlanOperation> MyComplexOperator::parse(Json::Value& data) {
     return std::make_shared<MyComplexOperator>(data["param"].asString());
   }

The query parser will call the `parse` method and pass the operators
JSON for custom parsing. The method has to return an instance of the
operator to be executed.

