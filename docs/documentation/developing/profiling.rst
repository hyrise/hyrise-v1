#########################
Profiling plan operations
#########################


In order to allow for profiling plans/plan operations, HYRISE features
two plan operations to start and stop profiling, ``StartProfiling``
and ``StopProfiling``.

To allow for profiling to work, you need to add
``USE_GOOGLE_PROFILING:=1`` in your ``settings.mk``. With this
parameter set, the respective operations will start and stop profiling
across all threads of HYRISE. Otherwise, the profiling ops will only
forward their input to their output.

See the following example for how to integrate them into your plan:

.. literalinclude: ../../test/autojson/howto_profiling.json
   :language: javascript

The profiling results are stored in the current working directory as
`profiling_<timestamp>.gprof`` and can be analysed via::

  $ google-pprof --text <binary> profiling_<timestamp>.gprof

Binary has to be replaced with the binary executable file that was
used to run your operations, usually this will be
``build/hyrise_server``.

When your profiled operations are relatively small and you only
collect a small number of samples per execution, you can also execute
the same query multiple times and analyze all the traces together by
running::

  $ google-pprof --text <binary> profiling_*.gprof
