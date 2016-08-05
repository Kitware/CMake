ninja-directory-targets
-----------------------

* The :generator:`Ninja` generator learned to produce phony targets
  of the form ``sub/dir/{test,install,package}`` to drive the build
  of a subdirectory installation, test or packaging target.
  This is equivalent to ``cd sub/dir; make {test,install,package}``
  with :ref:`Makefile Generators`.
