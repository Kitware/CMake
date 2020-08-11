cmake_language
--------------

Call meta-operations on CMake commands.

Synopsis
^^^^^^^^

.. parsed-literal::

  cmake_language(`CALL`_ <command> [<args>...])
  cmake_language(`EVAL`_ CODE <code>...)

Introduction
^^^^^^^^^^^^

This command will call meta-operations on built-in CMake commands or
those created via the :command:`macro` or :command:`function` commands.

``cmake_language`` does not introduce a new variable or policy scope.

Calling Commands
^^^^^^^^^^^^^^^^

.. _CALL:

.. code-block:: cmake

  cmake_language(CALL <command> [<args>...])

Calls the named ``<command>`` with the given arguments (if any).
For example, the code:

.. code-block:: cmake

  set(message_command "message")
  cmake_language(CALL ${message_command} STATUS "Hello World!")

is equivalent to

.. code-block:: cmake

  message(STATUS "Hello World!")

.. note::
  To ensure consistency of the code, the following commands are not allowed:

  * ``if`` / ``elseif`` / ``else`` / ``endif``
  * ``while`` / ``endwhile``
  * ``foreach`` / ``endforeach``
  * ``function`` / ``endfunction``
  * ``macro`` / ``endmacro``

Evaluating Code
^^^^^^^^^^^^^^^

.. _EVAL:

.. code-block:: cmake

  cmake_language(EVAL CODE <code>...)

Evaluates the ``<code>...`` as CMake code.

For example, the code:

.. code-block:: cmake

  set(A TRUE)
  set(B TRUE)
  set(C TRUE)
  set(condition "(A AND B) OR C")

  cmake_language(EVAL CODE "
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
