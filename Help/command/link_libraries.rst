link_libraries
--------------

Link libraries to all targets added later.

.. code-block:: cmake

  link_libraries([item1 [item2 [...]]]
                 [[debug|optimized|general] <item>] ...)

Specify libraries or flags to use when linking any targets created later in
the current directory or below by commands such as :command:`add_executable`
or :command:`add_library`.  See the :command:`target_link_libraries` command
for meaning of arguments.

.. |command_name| replace:: ``link_libraries``
.. |target_command_name| replace:: :command:`target_link_libraries`
.. note::
  .. include:: include/NON_TARGET_NOTE_BODY.rst

  Library dependencies are chained automatically, so directory-wide
  specification of link libraries is rarely needed.
