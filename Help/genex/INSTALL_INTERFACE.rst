INSTALL_INTERFACE
-----------------

.. |genex-family| replace:: :ref:`genex-family-export-and-install-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<INSTALL_INTERFACE:...>

Content of ``...`` when the property is exported using
:command:`install(EXPORT)`, and empty otherwise.
