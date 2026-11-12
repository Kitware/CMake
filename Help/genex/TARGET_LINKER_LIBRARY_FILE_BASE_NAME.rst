TARGET_LINKER_LIBRARY_FILE_BASE_NAME
------------------------------------

.. |genex-family| replace:: :ref:`genex-family-target-artifacts`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_LINKER_LIBRARY_FILE_BASE_NAME:tgt[,POSTFIX:(INCLUDE|EXCLUDE)]>

.. versionadded:: 3.27

.. versionadded:: 4.2
  The ``POSTFIX`` option can be used to control the inclusion or not
  of the :prop_tgt:`<CONFIG>_POSTFIX` target property value as part of the
  base name. The default is ``POSTFIX:INCLUDE``.

Base name of library file used to link the target ``tgt``, i.e.
:genex:`$<TARGET_LINKER_LIBRARY_FILE_NAME:tgt>` without prefix and suffix,and,
optionally, postfix. For example, if the target file name is
``libbase_postfix.a``, the base name is:

  * ``base_postfix`` for ``$<TARGET_LINKER_LIBRARY_FILE_BASE_NAME:tgt>`` or
    ``$<TARGET_LINKER_LIBRARY_FILE_BASE_NAME:tgt,POSTFIX:INCLUDE>``.
  * ``base`` for
    ``$<TARGET_LINKER_LIBRARY_FILE_BASE_NAME:tgt,POSTFIX:EXCLUDE>``.

See also the :prop_tgt:`OUTPUT_NAME`, :prop_tgt:`ARCHIVE_OUTPUT_NAME`, and
:prop_tgt:`LIBRARY_OUTPUT_NAME` target properties, their configuration-specific
variants :prop_tgt:`OUTPUT_NAME_<CONFIG>`,
:prop_tgt:`ARCHIVE_OUTPUT_NAME_<CONFIG>` and
:prop_tgt:`LIBRARY_OUTPUT_NAME_<CONFIG>`, and the :prop_tgt:`<CONFIG>_POSTFIX`
and :prop_tgt:`DEBUG_POSTFIX` target properties.

Note that ``tgt`` is not added as a dependency of the target this expression is
evaluated on.
