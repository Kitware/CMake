TARGET_OBJECTS
--------------

.. versionadded:: 3.1

.. |genex-family| replace:: :ref:`genex-family-target-artifacts`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<TARGET_OBJECTS:tgt[,SOURCE_FILES:source_file[;source_file]...]>

List of objects resulting from building ``tgt``.  This would typically be used
on :ref:`object library <Object Libraries>` targets.

Additional arguments:

``SOURCE_FILES:source_file[;source_file]...``

  .. versionchanged:: 4.4

  An optional :ref:`semicolon-separated list <CMake Language Lists>`  of one
  or more source files for the target. Only the object files built from those
  source files will be returned (but not necessarily in the same order they
  were specified).
