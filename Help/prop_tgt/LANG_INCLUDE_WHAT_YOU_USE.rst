<LANG>_INCLUDE_WHAT_YOU_USE
---------------------------

.. versionadded:: 3.3

This property is implemented only when ``<LANG>`` is ``C`` or ``CXX``.

Specify a :ref:`semicolon-separated list <CMake Language Lists>` containing a command
line for the ``include-what-you-use`` tool.  The :ref:`Makefile Generators`
and the :generator:`Ninja` generator will run this tool along with the
compiler and report a warning if the tool reports any problems.

This property is initialized by the value of
the :variable:`CMAKE_<LANG>_INCLUDE_WHAT_YOU_USE` variable if it is set
when a target is created.

.. versionadded:: 3.27

  This property supports
  :manual:`generator expressions <cmake-generator-expressions(7)>`.

  :prop_sf:`SKIP_LINTING` can be set on individual source files to exclude
  them from the linting process, which includes tools like
  :prop_tgt:`<LANG>_CPPLINT`, :prop_tgt:`<LANG>_CLANG_TIDY`,
  :prop_tgt:`<LANG>_CPPCHECK`, and :prop_tgt:`<LANG>_INCLUDE_WHAT_YOU_USE`.
  When :prop_sf:`SKIP_LINTING` is set on a source file, the mentioned tools
  will not be run on that specific file.
