PATH:HAS_*
----------

.. genex:: $<PATH:HAS_*,path>
   :target: PATH

.. |genex-family| replace:: :ref:`genex-family-genex-path-queries`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<PATH:HAS_*,path>

.. versionadded:: 3.24

The following operations return ``1`` if the particular path component is
present, ``0`` otherwise. See :ref:`Path Structure And Terminology` for the
meaning of each path component.

.. code-block:: cmake

  $<PATH:HAS_ROOT_NAME,path>
  $<PATH:HAS_ROOT_DIRECTORY,path>
  $<PATH:HAS_ROOT_PATH,path>
  $<PATH:HAS_FILENAME,path>
  $<PATH:HAS_EXTENSION,path>
  $<PATH:HAS_STEM,path>
  $<PATH:HAS_RELATIVE_PART,path>
  $<PATH:HAS_PARENT_PATH,path>

Note the following special cases:

* For ``HAS_ROOT_PATH``, a true result will only be returned if at least one
  of ``root-name`` or ``root-directory`` is non-empty.

* For ``HAS_PARENT_PATH``, the root directory is also considered to have a
  parent, which will be itself.  The result is true except if the path
  consists of just a :ref:`filename <FILENAME_DEF>`.
