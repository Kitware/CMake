PATH:RELATIVE_PATH
------------------

Syntax
^^^^^^

.. code-block:: cmake

  $<PATH:RELATIVE_PATH,path...,base_directory>

.. versionadded:: 3.24

Returns ``path``, modified to make it relative to the ``base_directory``
argument.

See :command:`cmake_path(RELATIVE_PATH)` for more details.
