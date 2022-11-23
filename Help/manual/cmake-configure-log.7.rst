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
      kind: "..."
      # (other fields omitted)
    -
      kind: "..."
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

Additional mapping keys are specific to each event kind.
