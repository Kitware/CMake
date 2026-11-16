PATH:NORMAL_PATH
----------------

.. versionadded:: 3.24

.. |genex-family| replace:: :ref:`genex-family-genex-path-transformations`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<PATH:NORMAL_PATH,path...>

Returns ``path`` normalized according to the steps described in
:ref:`Normalization`.
