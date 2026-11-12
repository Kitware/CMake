JOIN
----

.. |genex-family| replace:: :ref:`genex-family-genex-list-transformations`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<JOIN:list,glue>

Joins the ``list`` with the content of the ``glue`` string inserted between each
item.  This is conceptually the same operation as
:cref:`$\<LIST:JOIN,list,glue\> <GenEx LIST-JOIN>`, but the two have different
behavior with regard to empty items. :cref:`$\<LIST:JOIN,list,glue\> <GenEx
LIST-JOIN>` preserves all empty items, whereas ``$<JOIN:list,glue>`` drops all
empty items from the list.
