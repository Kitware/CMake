CMAKE_CROSSCOMPILING_EMULATOR
-----------------------------

.. versionadded:: 3.3

This variable is only used when :variable:`CMAKE_CROSSCOMPILING` is on. It
should point to a command on the host system that can run executable built
for the target system.

.. versionadded:: 3.15
  If this variable contains a :ref:`semicolon-separated list <CMake Language
  Lists>`, then the first value is the command and remaining values are its
  arguments.

.. versionadded:: 3.28
  This variable can be initialized via an
  :envvar:`CMAKE_CROSSCOMPILING_EMULATOR` environment variable.

The command will be used to run :command:`try_run` generated executables,
which avoids manual population of the ``TryRunResults.cmake`` file.

It is also used as the default value for the
:prop_tgt:`CROSSCOMPILING_EMULATOR` target property of executables.
