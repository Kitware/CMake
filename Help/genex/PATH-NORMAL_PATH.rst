PATH:NORMAL_PATH
----------------

.. |genex-family| replace:: :ref:`genex-family-genex-path-transformations`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<PATH:NORMAL_PATH,path...>

.. versionadded:: 3.24

Returns ``path`` normalized according to the steps described in
:ref:`Normalization`.
