MAKE_C_IDENTIFIER
-----------------

.. |genex-family| replace::
   :ref:`genex-family-string-transforming-generator-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<MAKE_C_IDENTIFIER:string>

Content of ``string`` converted to a C identifier.  The conversion follows the
same behavior as :command:`string(MAKE_C_IDENTIFIER)`.
