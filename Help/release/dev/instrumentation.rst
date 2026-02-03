instrumentation
---------------

* :manual:`cmake-instrumentation(7)` introduces a new API for the collection of
  timing data, target information and system diagnostic information during the
  configure, generate, build, test and install steps for a CMake project.
* When CMake instrumentation is enabled, instrumentation data will be sent
  as part of submissions to CDash.
* CMake can generate Google Trace Event Format files to visualize
  instrumentation data.
* The :command:`cmake_instrumentation` was added for adding project-level
  instrumentation queries.
