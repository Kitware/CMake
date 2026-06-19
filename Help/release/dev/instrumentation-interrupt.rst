instrumentation-interrupt
-------------------------

* The :manual:`cmake-instrumentation(7)` data version has been updated to 1.2.
* :manual:`cmake-instrumentation(7)` API now records an overall ``cmakeBuild``
  snippet even when a :option:`cmake --build` invocation is interrupted by the
  user (for example with Ctrl+C).  The snippet includes a new
  ``interruptSignal`` field, recording the signal that interrupted the build,
  so that consumers can distinguish an interrupted build from one that ran
  to completion.
