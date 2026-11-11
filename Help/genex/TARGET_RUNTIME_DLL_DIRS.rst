TARGET_RUNTIME_DLL_DIRS
-----------------------

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_RUNTIME_DLL_DIRS:tgt>

.. versionadded:: 3.27

List of the directories which contain the DLLs that the target depends on at
runtime (see :genex:`TARGET_RUNTIME_DLLS`). This is determined by the locations
of all the ``SHARED`` targets in the target's transitive dependencies. Using
this generator expression on targets other than executables, ``SHARED``
libraries, and ``MODULE`` libraries is an error. **On non-DLL platforms, this
expression always evaluates to an empty string**.

This generator expression can e.g. be used to create a batch file using
:command:`file(GENERATE)` which sets the PATH environment variable accordingly.
