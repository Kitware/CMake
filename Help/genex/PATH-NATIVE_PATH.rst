PATH:NATIVE_PATH
----------------

.. versionadded:: 4.0

.. |genex-family| replace:: :ref:`genex-family-genex-path-transformations`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<PATH:NATIVE_PATH[,NORMALIZE],path...>

Returns ``path`` converted into a native format with platform-specific slashes
(``\`` on Windows hosts and ``/`` elsewhere).

When the ``NORMALIZE`` option is specified, the path is :ref:`normalized
<Normalization>` before the conversion.

.. versionchanged:: 3.27
  CMake now accepts a list of paths as argument. When a list of paths is
   specified, the operation will be applied to each path.
