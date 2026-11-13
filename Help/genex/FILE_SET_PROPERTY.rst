FILE_SET_PROPERTY
-----------------

.. |genex-family| replace:: :ref:`genex-family-fileset-dependent-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<FILE_SET_PROPERTY:fileset,TARGET:target,prop>

.. versionadded:: 4.3

Value of the property ``prop`` on the file set ``fileset``, or empty if the
property is not set. An error will be raised if the file set is not known by
CMake.

The possible sub-options are:

``TARGET:target``
  The target on which the file set depends.
