cmake-job-pool
--------------

* A :variable:`CMAKE_JOB_POOLS` variable was added specify a value to use for
  the :prop_gbl:`JOB_POOLS` property. This enables control over build
  parallelism with command line configuration parameters when using the Ninja
  generator.
