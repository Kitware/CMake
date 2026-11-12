FILE_SET_EXISTS
---------------

.. |genex-family| replace:: :ref:`genex-family-fileset-dependent-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<FILE_SET_EXISTS:fileset,TARGET:target>

.. versionadded:: 4.3

``1`` if the ``fileset`` exists as a CMake file set attached to the ``target``,
else ``0``.

The possible sub-options are:

``TARGET:target``
  The target on which the file set depends.
