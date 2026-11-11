PATH:IS_RELATIVE
----------------

Syntax
^^^^^^

.. code-block:: cmake

  $<PATH:IS_RELATIVE,path>

.. versionadded:: 3.24

Returns ``1`` if the path is relative according to
:command:`cmake_path(IS_RELATIVE)`, ``0`` otherwise.
