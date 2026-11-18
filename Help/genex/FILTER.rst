FILTER
------

.. versionadded:: 3.15

.. |genex-family| replace:: :ref:`genex-family-genex-list-transformations`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<FILTER:list,INCLUDE|EXCLUDE,regex>

Includes or removes items from ``list`` that match the regular expression
``regex``.  The result is the same as
:genex:`$<LIST:FILTER,list,INCLUDE|EXCLUDE,regex> <LIST:FILTER>`.
