PATH:NATIVE_PATH
----------------

.. |genex-family| replace:: :ref:`genex-family-genex-path-transformations`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<PATH:NATIVE_PATH[,NORMALIZE],path...>

.. versionadded:: 4.0

Returns ``path`` converted into a native format with platform-specific slashes
(``\`` on Windows hosts and ``/`` elsewhere).

When the ``NORMALIZE`` option is specified, the path is :ref:`normalized
<Normalization>` before the conversion.
