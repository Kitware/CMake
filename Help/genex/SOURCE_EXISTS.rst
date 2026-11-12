SOURCE_EXISTS
-------------

.. |genex-family| replace:: :ref:`genex-family-source-dependent-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<SOURCE_EXISTS:src[,(DIRECTORY:dir|TARGET_DIRECTORY:tgt)]>

.. versionadded:: 4.3

``1`` if ``src`` exists as a CMake source file, else ``0``. By default, the
source file is searched in the scope of the current source directory or the
directory of the consuming target.

Directory scope can be overridden with one of the following sub-options:

``DIRECTORY:dir``
  The source file will be searched in the ``dir`` directory's scope.
  CMake must know about the directory, either by having added  it through a
  call to :command:`add_subdirectory` or ``dir`` being the top level
  directory. Relative paths are treated as relative to the current source
  directory.

``TARGET_DIRECTORY:target``
    The source file will be searched in the directory scope in which
    ``target`` was created (``target`` must therefore exist).
