Configure Log
-------------

* CMake now writes a YAML log of configure-time checks.
  See the :manual:`cmake-configure-log(7)` manual.

* The :manual:`cmake-file-api(7)` gained a new "configureLog" object kind
  that enables stable access to the :manual:`cmake-configure-log(7)`.

* The :command:`message` command gained a ``CONFIGURE_LOG`` mode to
  record an entry in the :manual:`cmake-configure-log(7)`.

* The :command:`try_compile` and :command:`try_run` commands gained
  a ``LOG_DESCRIPTION`` option specifying text to be recorded in the
  :manual:`cmake-configure-log(7)`.

* The :command:`try_compile` and :command:`try_run` commands gained a
  ``NO_LOG`` option to skip recording a :manual:`cmake-configure-log(7)`
  entry.
