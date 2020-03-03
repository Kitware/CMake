cmake_command
-------------

Call meta-operations on CMake commands.

Synopsis
^^^^^^^^

.. parsed-literal::

  cmake_command(`INVOKE`_ <command> [<args>...])
  cmake_command(`EVAL`_ CODE <code>...)

Introduction
^^^^^^^^^^^^

This command will call meta-operations on built-in CMake commands or
those created via the :command:`macro` or :command:`function` commands.

``cmake_command`` does not introduce a new variable or policy scope.

Invoking Commands
^^^^^^^^^^^^^^^^^

.. _INVOKE:

.. code-block:: cmake

  cmake_command(INVOKE <command> [<args>...])

Invokes the named ``<command>`` with the given arguments (if any).
For example, the code:

.. code-block:: cmake

  set(message_command "message")
  cmake_command(INVOKE ${message_command} STATUS "Hello World!")

is equivalent to

.. code-block:: cmake

  message(STATUS "Hello World!")

Evaluating Code
^^^^^^^^^^^^^^^

.. _EVAL:

.. code-block:: cmake

  cmake_command(EVAL CODE <code>...)

Evaluates the ``<code>...`` as CMake code.

For example, the code:

.. code-block:: cmake

  set(A TRUE)
  set(B TRUE)
  set(C TRUE)
  set(condition "(A AND B) OR C")

  cmake_command(EVAL CODE "
    if (${condition})
      message(STATUS TRUE)
    else()
      message(STATUS FALSE)
    endif()"
  )

is equivalent to

.. code-block:: cmake

  set(A TRUE)
  set(B TRUE)
  set(C TRUE)
  set(condition "(A AND B) OR C")

  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/eval.cmake "
    if (${condition})
      message(STATUS TRUE)
    else()
      message(STATUS FALSE)
    endif()"
  )

  include(${CMAKE_CURRENT_BINARY_DIR}/eval.cmake)
