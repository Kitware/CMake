cmake_command
-------------

Call meta-operations on CMake commands.

Synopsis
^^^^^^^^

.. parsed-literal::

  cmake_command(`INVOKE`_ <command> [<args>...])

Introduction
^^^^^^^^^^^^

This command will call meta-operations on built-in CMake commands or
those created via the :command:`macro` or :command:`function` commands.

Invoking
^^^^^^^^

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
