cmake-profiling
---------------

* Add support for profiling of CMake scripts through the parameters
  ``--profiling-output`` and ``--profiling-format``. These options can
  be used by users to gain insight into the performance of their scripts.

  The first supported output format is ``google-trace`` which is a format
  supported by Google Chrome's ``about:tracing`` tab.
