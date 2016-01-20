cmake-E-time-quoting
--------------------

* The :manual:`cmake(1)` ``-E time`` command now properly passes arguments
  with spaces or special characters through to the child process.  This
  may break scripts that worked around the bug with their own extra
  quoting or escaping.
