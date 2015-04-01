makefile-DELETE_ON_ERROR
------------------------

* The Makefile generators now add ``.DELETE_ON_ERROR`` to the
  makefiles that contain the actual build rules for files on disk.
  This tells GNU make to remove rule outputs when their recipe
  modifies an output but fails.
