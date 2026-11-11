PATH:IS_ABSOLUTE
----------------

Syntax
^^^^^^

.. code-block:: cmake

  $<PATH:IS_ABSOLUTE,path>

.. versionadded:: 3.24

Returns ``1`` if the path is absolute according to
:command:`cmake_path(IS_ABSOLUTE)`, ``0`` otherwise.
