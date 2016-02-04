error-multiple-targets
----------------------

* The :manual:`cmake(1)` ``--build`` command-line tool now rejects multiple
  ``--target`` options with an error instead of silently ignoring all but the
  last one.
