add_custom_command-depends-path
-------------------------------

* The :command:`add_custom_command` command learned to detect paths in
  ``DEPENDS`` arguments and convert them to paths relative to the current
  binary directory. This only applies to paths which contain a ``/`` or ``\\``
  in them because names like ``filename.txt`` could also be target names and
  cannot be coverted into absolute paths blindly.
