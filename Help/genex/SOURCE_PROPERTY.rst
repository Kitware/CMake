SOURCE_PROPERTY
---------------

Syntax
^^^^^^

.. code-block:: cmake

  $<SOURCE_PROPERTY:src[,(DIRECTORY:dir|TARGET_DIRECTORY:target)],prop>

.. versionadded:: 4.3

Value of the property ``prop`` on the source file ``src``, or empty if the
property is not set. An error will be raised if the source file is not known by
CMake. By default, the source file's property will be read from the current
source directory's scope or the directory of the consuming target.

Directory scope can be overridden with one of the following sub-options:

``DIRECTORY:dir``
  The source file property will be read from the ``dir`` directory's scope.
  CMake must know about the directory, either by having added  it through a
  call to :command:`add_subdirectory` or ``dir`` being the top level
  directory. Relative paths are treated as relative to the current source
  directory.

``TARGET_DIRECTORY:target``
    The source file property will be read from the directory scope in which
    ``target`` was created (``target`` must therefore exist).
