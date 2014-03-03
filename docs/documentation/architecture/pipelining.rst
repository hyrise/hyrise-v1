###########
Pipelining
###########

Implementation Idea
===================

The underlying idea consists of spawning tasks from the original tasks on receipt of new chunks.                                                                                                      
The main ideas for the implementation manifest itself in two mixins: the ``PipelineEmitter`` and the ``PipelineObserver``.                                                                  
These are described in the respective subsections.                                                                                                                                                    
We use a fictitious query execution plan featuring a pipeline to describe the implementation of the involved operators.                                                                               
This section concludes with a detailed description of the interaction between different pipeline components over the lifetime of a pipeline.

Interfaces vs. Mixins
=====================

White initially all functionality was implemented in interfaces, it soon became apparent that the implementation was almost identical for all derived classes.
Some of the implementation depends on the ``PlanOperation`` and ``Task`` interfaces, but  we did not want our interface to derive from ``PlanOperation``.
Therefore, we decided to use the Curiously Recurring Template Pattern (CRTP).
This pattern allowed us to express the mixin nature of both ``PipelineEmitter`` as well ``PipelineObserver`` more naturally. 
By using CRTP, the implementation of new pipeline-enabled operation becomes less convoluted and involves less boilerplate code. 

PipelineEmitter Mixin
======================

The ``PipelineEmitter`` mixin represents the functionality necessary to send chunks to dependent PipelineObservers.
This mixin offers one function: 

  ``virtual void emitChunk(c_aresource_ptr_t chunk)``

Depending on the nature of the base operation, the result will either be a table or a hash table.
The ``executePlanOperation()`` method, a ``PlanOperation``'s main method, has to be altered so that intermediate results are produced and then forwarded to all succeeding, dependent pipelining operations, i.e. operations derived from ``PipelineObserver``. 
The implementor of the pipeline-enabled operation still has to decide on when and how to emit intermediary results of the requested chunk size.
The mixin implements the functionality to find the dependent, succeeding operations and to forward chunks to these operations.


PipelineObserver Mixin
======================

The ``PipelineObserver`` mixin encapsulates all functionality for receiving chunks.
The main function mixed in by the ``PipelineObserver`` mixin is 

  ``virtual void notifyNewChunk(c_aresource_ptr_t chunkTbl)`` and 

Both method implementations are based on the main idea of our pipelining approach:
**First**, they will produce a copy of the original task.
Therefore, the original task will never be executed and acts as a template only.
The actual processing will be carried out by the copies. 
**Second**, the input of the copy is set to the chunk passed to the ``notifyNew(Hash)Chunk`` method.
**Third**, the new copy task will be set as a dependency to all tasks that the template task is a dependency of.
Therefore, the dependent tasks can only start running if and only if all copied tasks have finished running.
**Fourth** and finally, the newly copied task will be scheduled to run.
Since the copied task itself has no dependencies, it is immediately ready and could be run.

*Pipeline breakers* depend on more than one input.
That requires that chunk tasks wait for certain dependencies of the template
task.
This, for example, is the case for a ``PipeliningHashProbe``;
the probe table's chunks might arrive before the hash table of the other relation is built.
Therefore, the chunk tasks have the hash build task as a dependency.
The ``PipelineObserver`` mixin offers the virtual ``setCustomDependencies`` method to explicitly set additional dependencies.

Pipelining Terminators
======================


.. image:: /_static/pipelining/example_qep.png

The previous figure shows a contrived example of a query execution plan that we will use to give a step by step example of how the pipeline implementation works.
One inherent aspect of pipelining is that we need to terminate the pipeline at some point and convert all chunks into one final blocking result table again.
Therefore, we introduce the notion of ``pipeline terminators``.

These are operations that will not copy themselves on the receipt of chunks but rather just collect chunks and merge them after receipt of the final chunk.
In terms of our implementation these are operators that only inherit from ``PipelineObserver`` but not from ``PipelineEmitter``.
For the collection of table chunks we offer a ``PipelineUnion``, for the collection of hash table chunks we offer a ``PipelineMergeHashTables``.
Assuming operation ``D`` of the pipeline from the previous figure produces table chunks, the query execution plan with pipelining enabled would look like the following: 

.. image:: /_static/pipelining/example_qep_terminated.png

Pipelining Example
==================

In order to explain the operation of the pipeline, we will slice it into segments:
the head, between operations ``B`` and ``C``;
the middle, between operations ``C`` and ``D``;
and the tail, between operations ``D`` and the ``PipelineUnion`` ``PU``.
We will look at these different segments individually and conclude this section with a look at the task dependencies during runtime as well as the resulting tear-down process for the pipeline.

Pipeline Head
=============

.. image:: /_static/pipelining/pipeline_head_seq.png

The previous figure shows a simplified sequence diagram of the interactions taking place to start a pipeline.
The first operation of the pipeline has to derive from ``PipelineEmitter``.
The black vertical boxes in the figure represent execution of a member method of the respective class.
Operation ``A`` is a non-pipelining operation.
It will run the ``executePlanOperation()`` method and produce one final result.
On completion, ``A`` will notify its done observers.
A done observer is every task that had this task as a dependency.
Since ``B`` only has ``A`` as a dependency, it will become ready.
Now the plan operation ``B`` will execute its ``executePlanOperation()`` method.
In contrast to ``A``, ``B`` will not produce one final result but instead produces intermediary chunks.
These chunks are passed on to succeeding ``PipelineObservers`` by means of the ``emitChunk`` method inherited from the ``PipelineEmitter`` mixin.

Operation ``C`` inherits from ``PipelineObserver`` and ``PipelineEmitter``.
All operations contained in the middle of a pipeline have to inherit from both mixins to be able to function.
Upon receipt of a new chunk, the operation's task will act as a template and a copy is created.
The input of this copy is set to the received chunk.
The copy is set as a dependency to the following operation, ``D``.
This copy is immediately ready and can be scheduled to run.

All of the afore described behavior is implemented in ``notifyNewChunk`` of the ``PipelineObserver`` mixin and does not need to implemented on a per-operation basis.
If an operation is dependent on more than the input of the chunk, the virtual ``addCustomDependencies()`` method can be overridden with setting additional dependencies in the derived classes.
E.g., the PipeliningHashProbe uses this to set a dependency to the hash build.

Pipeline Middle
===============

.. image:: /_static/pipelining/pipeline_middle_seq.png

When the created copies are ready, their main execution method ``executePlanOperation()`` will process the chunk and produce output (cf. the above figure).
The output are chunks again and will be forwarded to the following ``PipelineObservers``.
Depending on the operation and the chunk size, a single chunk might produce more than one output chunk.
Aggregation of single chunks to avoid fragmentation of chunks throughout the pipeline is not implemented at the moment and would prove to be difficult with the current architecture.
As a single chunk task could not output chunks directly but rather the template operator would need to act as a central container for all produced chunks.
A sample implementation was tried and proved to be too convoluted with no observed real benefit.

On receipt of chunks, operation ``D`` will -- like any other PipelineObserver -- act as a template.
The resulting task is then scheduled to run.

Pipeline Tail
=============

.. image:: /_static/pipelining/pipeline_tail_seq.png

The ``PipelineUnion`` is still a ``PipelineObserver``, but unlike other observers it does not utilize the standard template-copy behavior provided by the mixin.
Instead, it overwrites the ``notifyNewChunk`` method and stores all incoming chunks.
As soon as all chunks are received, i.e. all preceding chunk tasks have finished running their ``executePlanOperation`` methods, the ``PipelineUnion`` operator will merge all stored chunks into a single result during the its ``executePlanOperation`` method.
The same applies to the ``PipelineMergeHashTables`` operator.
Both pipeline-terminating operators could also have been implemented in an incremental fashion, i.e. instead of storing the incoming chunks, they would repeatedly merge chunks with an accumulated result.
This did not prove to be efficient in Hyrise since both operations are implemented in such a way that incremental merging will result in more overall work;
in the case of hash tables, merging hash tables is the same as reconstructing hash tables.
For this reason, a simple *store-and-delay-merging* approach was favored.

Pipeline Termination
====================

Since most of the operator's tasks in a pipeline act as templates for chunk tasks, the layout of a query execution plan changes over its runtime.
Looking at the dependency tree for the running example of this section, this becomes apparent.
This shows the initial dependency tree of the query:

.. image:: /_static/pipelining/parse_dependencies.png

This shows the dependency tree after three chunks have passed through the pipeline without further fragmentation:

.. image:: /_static/pipelining/runtime_dependencies.png

All observers have their preceding chunk tasks set as new dependencies.
In the case of the preceding figure, ``D`` and ``PU`` both have to wait for the completion of ``executePlanOperation`` in the respective chunk tasks as well as in the preceding template task.
We need to prove that this model is sufficient to have the final ``executePlanOperation`` of the pipeline-terminating operation executed iff all chunks have been received:
A plan operations' main method runs iff the main method of all dependencies have completed their execution.
This statement can also be formulated in a slightly different way, allowing for a proof by full, structural induction: 

  The template's main method will run iff all chunks caused a ``notifyNewChunk`` call.

The minimal pipeline consists of two operations:
A head operation, which takes a blocking result and emits chunks during its execution of the main method; and a tail operation, which receives chunks, stores them, and reassembles a final blocking result during the execution of its main method.
The main operation of the second operation will only run iff the main operation of the first operation concluded.
The main operation of the first operation will only finish once all chunks have been emitted and forwarded via a ``notifyNewChunk`` call.
Therefore, the main operation of the second operation will only run if all chunks have been sent and received.
Thus, a base pipeline obeys our aforementioned statement.

Suppose all pipelines of length *n* or smaller obey our statement.
We can extend the pipeline by one operation with the insertion of a new operation just in front of the pipeline terminator.
The pipeline terminator's main method will run if the main method of the newly inserted operation has run.
This main method will run iff the preceding operators main method has run.
Since the preceding operator is also part of a smaller pipeline, it obeys our statement per assumption.
Therefore, all chunks have been emitted and caused the necessary ``notifyNewChunk`` calls in the preceding operation.
The main method of the newly inserted operation will run iff all the dependencies, which include the chunk tasks, have finished their execution and forwarded all chunks.
Hence, our newly constructed pipeline obeys the corollary as well.
This inductive nature of the system can also be seen in the following figure.

.. image:: /_static/pipelining/pipeline_terminate.png

This simplified sequence diagram shows how a single chunk would pass through the pipeline.
Operator ``B`` produces the chunk in its main method.
The chunk will create a copy of the task of operator ``C`` and register it as a dependency to operator ``D``.
Then, ``B`` will finish executing its main method and notify the template task of ``C``.
``C``'s main method will run since all the dependencies are done and notify the template task of ``D``.
``D``'s main method cannot yet run as the chunk task dependency for ``C`` is not done.
The chunk task of ``C`` will emit a new chunk and forward it.
This creates a chunk task for ``D``, which will be registered as a dependency of the ``PipelineUnion``.
The chunk task for ``C`` finishes its main method.
This causes the main method of ``D`` to run.
In turn, ``PipelineUnion`` cannot yet run as its final dependency, the chunk task for ``D`` has not yet finished.
After emitting a single chunk, the chunk task for ``D`` concludes the execution of its main method.
All dependencies of the ``PipelineUnion`` are now satisfied and the main method executes as the final operation of the pipeline.

