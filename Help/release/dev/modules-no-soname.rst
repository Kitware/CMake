modules-no-soname
-----------------

* The ``SONAME`` field is no longer set for ``MODULE`` libraries
  created with the :command:`add_library` command.  ``MODULE``
  libraries are meant for explicit dynamic loading at runtime.
  They cannot be linked so ``SONAME`` is not useful.
