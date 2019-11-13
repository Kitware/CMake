Command-Line
--------------------

* :manual:`cmake(1)` gained a ``rm`` command line
  option that can be used to remove directories (with ``-r`` or ``-R`` flag)
  and files.
  If the ``-f`` flag is not specified, attempting to remove a file that
  doesn't exist returns an non-zero error code.
  This command deprecates ``remove`` and ``remove_directory``.
  The ``remove`` implementation was buggy and always returned 0 when ``force``
  flag was not present and a file didn't exist. It cannot be fixed without
  breaking backwards compatibility so we introduced ``rm``.
