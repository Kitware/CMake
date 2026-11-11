LIST:JOIN
---------

Syntax
^^^^^^

.. code-block:: cmake

  $<LIST:JOIN,list,glue>

.. versionadded:: 3.27

Converts ``list`` to a single string with the content of the ``glue`` string
inserted between each item.  This is conceptually the same operation as
:genex:`$<JOIN:list,glue>`, but the two have different behavior with regard to
empty items.  ``$<LIST:JOIN,list,glue>`` preserves all empty items, whereas
:genex:`$<JOIN:list,glue>` drops all empty items from the list.
