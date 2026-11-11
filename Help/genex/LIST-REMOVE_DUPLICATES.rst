LIST:REMOVE_DUPLICATES
----------------------

Syntax
^^^^^^

.. code-block:: cmake

  $<LIST:REMOVE_DUPLICATES,list>

.. versionadded:: 3.27

The ``list`` with all duplicated items removed.  The relative order of items is
preserved, but if duplicates are encountered, only the first instance is
preserved.  The result is the same as :genex:`$<REMOVE_DUPLICATES:list>`.
