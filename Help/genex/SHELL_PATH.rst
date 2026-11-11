SHELL_PATH
----------

Syntax
^^^^^^

.. code-block:: cmake

  $<SHELL_PATH:...>

.. versionadded:: 3.4

Content of ``...`` converted to shell path style. For example, slashes are
converted to backslashes in Windows shells and drive letters are converted to
posix paths in MSYS shells. The ``...`` must be an absolute path.

.. versionadded:: 3.14
  The ``...`` may be a :ref:`semicolon-separated list <CMake Language Lists>`
  of paths, in which case each path is converted individually and a result
  list is generated using the shell path separator (``:`` on POSIX and
  ``;`` on Windows).  Be sure to enclose the argument containing this genex
  in double quotes in CMake source code so that ``;`` does not split arguments.
