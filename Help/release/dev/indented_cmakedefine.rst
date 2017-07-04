indented_cmakedefine
--------------------

* The :command:`configure_file` command learned to support indented
  ``#  cmakedefine`` and ``#  cmakedefine01``. Spaces and/or tabs between
  the ``#`` character and the ``cmakedefine``/``cmakedefine01`` words
  are now understood and preserved in the output.
