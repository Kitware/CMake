PATH:RELATIVE_PATH
------------------

.. |genex-family| replace:: :ref:`genex-family-genex-path-transformations`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<PATH:RELATIVE_PATH,path...,base_directory>

.. versionadded:: 3.24

Returns ``path``, modified to make it relative to the ``base_directory``
argument.

See :command:`cmake_path(RELATIVE_PATH)` for more details.
