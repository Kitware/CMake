CMAKE_HOST_SYSTEM_PROCESSOR
---------------------------

The name of the CPU CMake is running on.

On Windows, this variable is set to the value of the environment variable
``PROCESSOR_ARCHITECTURE``. On systems that support ``uname``, this variable is
set to the output of:

- ``uname -m`` on GNU, Linux, Cygwin, Darwin, Android, or
- ``arch`` on OpenBSD, or
- on other systems,

  * ``uname -p`` if its exit code is nonzero, or
  * ``uname -m`` otherwise.
