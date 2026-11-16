PATH:GET_PARENT_PATH
--------------------

.. versionadded:: 3.24

.. |genex-family| replace:: :ref:`genex-family-path-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<PATH:GET_PARENT_PATH,path...>

The following operations retrieve a different component or group of components
from a path. See :ref:`Path Structure And Terminology` for the meaning of each
path component.

.. versionchanged:: 3.27
  CMake now accepts a list of paths as argument. When a list of paths is
   specified, the operation will be applied to each path.

If a requested component is not present in the path, an empty string is
returned.
