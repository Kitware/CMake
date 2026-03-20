cmake_diagnostic
----------------

.. versionadded:: 4.4

Manage CMake Diagnostic settings.  See the :manual:`cmake-diagnostics(7)`
manual for a list of available categories.

Synopsis
^^^^^^^^

.. parsed-literal::

  `Setting Diagnostics`_
    cmake_diagnostic(`SET`_ <category> <action> [RECURSE])
    cmake_diagnostic(`PROMOTE`_ <category> <action> [NO_RECURSE])
    cmake_diagnostic(`DEMOTE`_ <category> <action> [NO_RECURSE])

  `Checking Diagnostic Actions`_
    cmake_diagnostic(`GET`_ <diagnostic> <out-var>)

  `CMake Diagnostic Stack`_
    cmake_diagnostic(`PUSH`_)
    cmake_diagnostic(`POP`_)

Setting Diagnostics
^^^^^^^^^^^^^^^^^^^

.. signature::
  cmake_diagnostic(SET CMD_<CATEGORY> <action> [RECURSE])
  cmake_diagnostic(PROMOTE CMD_<CATEGORY> <action> [NO_RECURSE])
  cmake_diagnostic(DEMOTE CMD_<CATEGORY> <action> [NO_RECURSE])
  :target:
    SET
    PROMOTE
    DEMOTE

Set or alter the action taken when a diagnostic belonging to a particular
category is triggered.

The ``SET`` subcommand sets the action for the specified diagnostic category.
The ``PROMOTE`` subcommand increases the severity for the specified diagnostic
category, or does nothing if the action was already set to an equal or higher
severity.  The ``DEMOTE`` subcommand decreases the severity for the specified
diagnostic category, or does nothing if the action was already set to an equal
or lower severity.

The possible ``<action>``\ s (in order of severity) are:

``IGNORE``
  Do nothing.

``WARN``
  Report a warning and continue processing.

``SEND_ERROR``
  Report an error, continue processing, but skip generation.

  The :manual:`cmake(1)` executable will return a non-zero
  :ref:`exit code <CMake Exit Code>`.

``FATAL_ERROR``
  Report an error, stop processing and generation.

  The :manual:`cmake(1)` executable will return a non-zero
  :ref:`exit code <CMake Exit Code>`.

Some diagnostic categories are hierarchical.  The ``RECURSE`` and
``NO_RECURSE`` options determine whether changing the action for a diagnostic
category also modifies any child categories.  By default, the ``PROMOTE`` and
``DEMOTE`` subcommands are recursive, while the ``SET`` subcommand is not.
Note that the alteration for child categories is independent of the prior
action set on any parents; that is, ``PROMOTE`` and ``DEMOTE``, when operating
recursively, will operate on all child categories even if a parent category's
action was not altered.

Checking Diagnostic Actions
^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. signature:: cmake_diagnostic(GET CMD_<CATEGORY> <variable>)
  :target: GET

Check what action is currently specified for a diagnostic category.
The output ``<variable>`` value will be one of ``IGNORE``, ``WARN``,
``SEND_ERROR`` or ``FATAL_ERROR``.

CMake Diagnostic Stack
^^^^^^^^^^^^^^^^^^^^^^

CMake keeps diagnostic settings on a stack, so changes made by the
``cmake_diagnostic`` command affect only the top of the stack.  A new entry on
the diagnostic stack is managed automatically for each subdirectory to protect
its parents and siblings.  CMake also manages a new entry for scripts loaded by
:command:`include` and :command:`find_package` commands except when invoked
with the ``NO_DIAGNOSTIC_SCOPE`` option.  The ``cmake_diagnostic`` command
provides an interface to manage custom entries on the diagnostic stack:

.. signature:: cmake_diagnostic(PUSH)

  Create a new entry on the diagnostic stack.

.. signature:: cmake_diagnostic(POP)

  Remove the last diagnostic stack entry created with
  ``cmake_diagnostic(PUSH)``.

Each ``PUSH`` must have a matching ``POP`` to erase any changes.
This is useful to make temporary changes to diagnostic settings.
Calls to the :command:`cmake_diagnostic(SET)`,
:command:`cmake_diagnostic(PROMOTE)`, or :command:`cmake_diagnostic(DEMOTE)`
commands influence only the current top of the diagnostic stack.

The :command:`block(SCOPE_FOR DIAGNOSTICS)` command offers a more flexible
and more secure way to manage the diagnostic stack. The pop action is done
automatically when leaving the block scope, so there is no need to
precede each :command:`return` with a call to :command:`cmake_diagnostic(POP)`.

.. code-block:: cmake

  # stack management with cmake_diagnostic()
  function(my_func)
    cmake_diagnostic(PUSH)
    cmake_diagnostic(SET ...)
    if (<cond1>)
      ...
      cmake_diagnostic(POP)
      return()
    elseif(<cond2>)
      ...
      cmake_diagnostic(POP)
      return()
    endif()
    ...
    cmake_diagnostic(POP)
  endfunction()

  # stack management with block()/endblock()
  function(my_func)
    block(SCOPE_FOR DIAGNOSTICS)
      cmake_diagnostic(SET ...)
      if (<cond1>)
        ...
        return()
      elseif(<cond2>)
        ...
        return()
      endif()
      ...
    endblock()
  endfunction()

Commands created by the :command:`function` and :command:`macro` commands
record diagnostic settings when they are created and use the pre-record
diagnostics when they are invoked.  If the function or macro implementation
sets diagnostics, the changes automatically propagate up through callers until
they reach the closest nested diagnostic stack entry.
