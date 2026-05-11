Diagnostics
-----------

* CMake diagnostic actions are now tracked in a new state type managed by the
  :command:`cmake_diagnostic` command.  This replaces the old system which
  sometimes used variables to manage reporting actions and sometimes did not
  allow management beyond command-line options and presets.  The new system
  also introduces several new diagnostic categories and significantly reduces
  the cost of adding additional categories in the future.  See the
  :manual:`cmake-diagnostics(7)` manual for a list of available categories.

Commands
--------

* The :command:`cmake_diagnostic` command was added to manipulate CMake's
  diagnostic state.

* The :command:`block` command can now create a ``DIAGNOSTICS`` scope.

* The :command:`include` command now has a ``NO_DIAGNOSTIC_SCOPE`` option.

Deprecated and Removed Features
-------------------------------

* The ``-W[no-][error=]dev`` command-line options are deprecated.  The new
  spelling is :option:`-W[no-][error=]author <cmake -W>`.

* The ``--warn-uninitialized`` command-line option is deprecated.  The new
  spelling is :option:`-Wuninitialized <cmake -W>`.

* The ``--no-warn-unused-cli`` command-line option is deprecated.  The new
  spelling is :option:`-Wno-unused-cli <cmake -Wno->`.

* The :variable:`CMAKE_WARN_DEPRECATED` and :variable:`CMAKE_ERROR_DEPRECATED`
  variables are deprecated.  The new :command:`cmake_diagnostic` command should
  be used instead.
