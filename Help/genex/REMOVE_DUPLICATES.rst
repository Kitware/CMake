REMOVE_DUPLICATES
-----------------

.. |genex-family| replace:: :ref:`genex-family-genex-list-transformations`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<REMOVE_DUPLICATES:list>

.. versionadded:: 3.15

Removes duplicated items in the given ``list``. The relative order of items is
preserved, and if duplicates are encountered, only the first instance is
retained.  The result is the same as :cref:`$\<LIST:REMOVE_DUPLICATES,list\>
<GenEx LIST-REMOVE_DUPLICATES>`.
