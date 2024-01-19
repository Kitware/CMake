find_library-msvc-libfoo.a
--------------------------

* On Windows, when targeting the MSVC ABI, the :command:`find_library` command
  now accepts ``.a`` file names after first considering ``.lib``.  This is
  symmetric with existing behavior when targeting the GNU ABI, in which the
  command accepts ``.lib`` file names after first considering ``.a``.
