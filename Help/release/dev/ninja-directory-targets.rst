ninja-directory-targets
-----------------------

* The :generator:`Ninja` generator learned to produce phony targets
  of the form ``sub/dir/all`` to drive the build of a subdirectory.
  This is equivalent to ``cd sub/dir; make all`` with
  :ref:`Makefile Generators`.
