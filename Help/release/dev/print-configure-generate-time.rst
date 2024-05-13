print-configure-generate-time
-----------------------------

* The durations printed after "Configuring done" and "Generating done"
  messages now reflect time spent in generator-specific steps, and
  in a code model evaluation step at the beginning of generation that
  was not previously captured.  Printed durations may appear longer
  than in previous versions of CMake.
