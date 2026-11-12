DIRECTORY_PROPERTY
------------------

.. |genex-family| replace:: :ref:`genex-family-generator-expression-reference`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<DIRECTORY_PROPERTY:dir,prop>

.. versionadded:: 4.5

Value of the property ``prop`` on the directory ``dir``, or empty if the
property is not set. An error will be raised if the directory is not known by
CMake.
