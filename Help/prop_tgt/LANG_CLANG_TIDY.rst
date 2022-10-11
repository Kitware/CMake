<LANG>_CLANG_TIDY
-----------------

.. versionadded:: 3.6

This property is implemented only when ``<LANG>`` is ``C``, ``CXX``, ``OBJC``
or ``OBJCXX``.

Specify a :ref:`semicolon-separated list <CMake Language Lists>` containing
a command line for the ``clang-tidy`` tool.  The :ref:`Makefile Generators`
and the :generator:`Ninja` generator will run this tool along with the
compiler and report a warning if the tool reports any problems.

The specified ``clang-tidy`` command line will be invoked with additional
arguments specifying the source file and, after ``--``, the full compiler
command line.

.. versionchanged:: 3.25

  If the specified ``clang-tidy`` command line includes the ``-p`` option,
  it will be invoked without ``--`` and the full compiler command line.
  ``clang-tidy`` will look up the source file in the specified compiler
  commands database.

This property is initialized by the value of
the :variable:`CMAKE_<LANG>_CLANG_TIDY` variable if it is set
when a target is created.
