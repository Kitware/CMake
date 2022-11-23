.. cmake-manual-description: CMake Configure Log

cmake-configure-log(7)
**********************

.. versionadded:: 3.26

.. only:: html

   .. contents::

Introduction
============

CMake writes a running log, known as the configure log,
of certain events that occur during the "configure" step.
The log file is located at::

  ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeConfigureLog.yaml

The configure log does *not* contain a log of all output, errors,
or messages printed while configuring a project.  It is a log of
detailed information about specific events, such as toolchain inspection
by :command:`try_compile`, meant for use in debugging the configuration
of a build tree.

Log Structure
=============

The configure log is designed to be both machine- and human-readable.

The log file is a YAML document stream containing zero or more YAML
documents separated by document markers.  Each document begins
with a ``---`` document marker line, contains a single YAML mapping
that logs events from one CMake "configure" step, and, if the configure
step finished normally, ends with a ``...`` document marker line:

.. code-block:: yaml

  ---
  version:
    major: 1
    minor: 0
  events:
    -
      kind: "try_compile"
      # (other fields omitted)
    -
      kind: "try_compile"
      # (other fields omitted)
  ...

A new document is appended to the log every time CMake configures
the build tree and logs new events.

The keys of the each document root mapping are:

``version``
  A YAML mapping that describes the schema version of the log document.
  It has keys ``major`` and ``minor`` holding non-negative integer values.

``events``
  A YAML block sequence of nodes corresponding to events logged during
  one CMake "configure" step.  Each event is a YAML node containing one
  of the `Event Kinds`_ documented below.

Text Block Encoding
-------------------

In order to make the log human-readable, text blocks are always
represented using YAML literal block scalars (``|``).
Since literal block scalars do not support escaping, backslashes
and non-printable characters are encoded at the application layer:

* ``\\`` encodes a backslash.
* ``\xXX`` encodes a byte using two hexadecimal digits, ``XX``.

.. _`configure-log event kinds`:

Event Kinds
===========

Every event kind is represented by a YAML mapping of the form:

.. code-block:: yaml

  kind: "<kind>"
  backtrace:
    - "<file>:<line> (<function>)"
  #...event-specific keys...

The keys common to all events are:

``kind``
  A string identifying the event kind.

``backtrace``
  A YAML block sequence reporting the call stack of CMake source
  locations at which the event occurred.  Each node is a string
  specifying one location formatted as ``<file>:<line> (<function>)``.

Additional mapping keys are specific to each event kind,
described below.

.. _`try_compile event`:

Event Kind ``try_compile``
--------------------------

The :command:`try_compile` command logs ``try_compile`` events.

A ``try_compile`` event is a YAML mapping:

.. code-block:: yaml

  kind: "try_compile"
  backtrace:
    - "CMakeLists.txt:123 (try_compile)"
  directories:
    source: "/path/to/.../TryCompile-01234"
    binary: "/path/to/.../TryCompile-01234"
  buildResult:
    variable: "COMPILE_RESULT"
    cached: true
    stdout: |
      # ...
    exitCode: 0

The keys specific to ``try_compile`` mappings are:

``directories``
  A mapping describing the directories associated with the
  compilation attempt.  It has the following keys:

  ``source``
    String specifying the source directory of the
    :command:`try_compile` project.

  ``binary``
    String specifying the binary directory of the
    :command:`try_compile` project.
    For non-project invocations, this is often the same as
    the source directory.

``buildResult``
  A mapping describing the result of compiling the test code.
  It has the following keys:

  ``variable``
    A string specifying the name of the CMake variable
    storing the result of trying to build the test project.

  ``cached``
    A boolean indicating whether the above result ``variable``
    is stored in the CMake cache.

  ``stdout``
    A YAML literal block scalar containing the output from building
    the test project, represented using our `Text Block Encoding`_.
    This contains build output from both stdout and stderr.

  ``exitCode``
    An integer specifying the build tool exit code from trying
    to build the test project.

Event Kind ``try_run``
----------------------

The :command:`try_run` command logs ``try_run`` events.

A ``try_run`` event is a YAML mapping:

.. code-block:: yaml

  kind: "try_run"
  backtrace:
    - "CMakeLists.txt:456 (try_run)"
  directories:
    source: "/path/to/.../TryCompile-56789"
    binary: "/path/to/.../TryCompile-56789"
  buildResult:
    variable: "COMPILE_RESULT"
    cached: true
    stdout: |
      # ...
    exitCode: 0
  runResult:
    variable: "RUN_RESULT"
    cached: true
    stdout: |
      # ...
    stderr: |
      # ...
    exitCode: 0

The keys specific to ``try_run`` mappings include those
documented by the `try_compile event`_, plus:

``runResult``
  A mapping describing the result of running the test code.
  It has the following keys:

  ``variable``
    A string specifying the name of the CMake variable
    storing the result of trying to run the test executable.

  ``cached``
    A boolean indicating whether the above result ``variable``
    is stored in the CMake cache.

  ``stdout``
    An optional key that is present when the test project built successfully.
    Its value is a YAML literal block scalar containing output from running
    the test executable, represented using our `Text Block Encoding`_.

    If ``RUN_OUTPUT_VARIABLE`` was used, stdout and stderr are captured
    together, so this will contain both.  Otherwise, this will contain
    only the stdout output.

  ``stderr``
    An optional key that is present when the test project built successfully
    and the ``RUN_OUTPUT_VARIABLE`` option was not used.
    Its value is a YAML literal block scalar containing output from running
    the test executable, represented using our `Text Block Encoding`_.

    If ``RUN_OUTPUT_VARIABLE`` was used, stdout and stderr are captured
    together in the ``stdout`` key, and this key will not be present.
    Otherwise, this will contain the stderr output.

  ``exitCode``
    An optional key that is present when the test project built successfully.
    Its value is an integer specifying the exit code, or a string containing
    an error message, from trying to run the test executable.
