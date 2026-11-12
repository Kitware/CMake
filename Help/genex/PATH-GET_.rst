PATH:GET_*
----------

.. |genex-family| replace:: :ref:`genex-family-path-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<PATH:GET_*,...>

.. versionadded:: 3.24

The following operations retrieve a different component or group of components
from a path. See :ref:`Path Structure And Terminology` for the meaning of each
path component.

.. versionchanged:: 3.27
  All operations now accept a list of paths as argument. When a list of paths
  is specified, the operation will be applied to each path.

.. code-block:: cmake

  $<PATH:GET_ROOT_NAME,path...>
  $<PATH:GET_ROOT_DIRECTORY,path...>
  $<PATH:GET_ROOT_PATH,path...>
  $<PATH:GET_FILENAME,path...>
  $<PATH:GET_EXTENSION[,LAST_ONLY],path...>
  $<PATH:GET_STEM[,LAST_ONLY],path...>
  $<PATH:GET_RELATIVE_PART,path...>
  $<PATH:GET_PARENT_PATH,path...>

If a requested component is not present in the path, an empty string is
returned.
