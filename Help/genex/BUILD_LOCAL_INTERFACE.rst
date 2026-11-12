BUILD_LOCAL_INTERFACE
---------------------

.. |genex-family| replace:: :ref:`genex-family-export-and-install-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<BUILD_LOCAL_INTERFACE:...>

.. versionadded:: 3.26

Content of ``...`` when the target is used by another target in the same
buildsystem. Expands to the empty string otherwise.
