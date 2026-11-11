TARGET_FILE_BASE_NAME
---------------------

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_FILE_BASE_NAME:tgt[,POSTFIX:(INCLUDE|EXCLUDE)]>

.. versionadded:: 3.15

.. versionadded:: 4.2
  The ``POSTFIX`` option can be used to control the inclusion or not
  of the :prop_tgt:`<CONFIG>_POSTFIX` target property value as part of the
  base name. The default is ``POSTFIX:INCLUDE``.

Base name of ``tgt``, i.e. ``$<TARGET_FILE_NAME:tgt>`` without prefix and suffix
and, optionally, postfix. For example, if the ``tgt`` filename is
``libbase_postfix.so``, the base name is:

  * ``base_postfix`` for ``$<TARGET_FILE_BASE_NAME:tgt>`` or
    ``$<TARGET_FILE_BASE_NAME:tgt,POSTFIX:INCLUDE>``.
  * ``base`` for ``$<TARGET_FILE_BASE_NAME:tgt,POSTFIX:EXCLUDE>``.

See also the :prop_tgt:`OUTPUT_NAME`, :prop_tgt:`ARCHIVE_OUTPUT_NAME`,
:prop_tgt:`LIBRARY_OUTPUT_NAME` and :prop_tgt:`RUNTIME_OUTPUT_NAME` target
properties, their configuration-specific variants
:prop_tgt:`OUTPUT_NAME_<CONFIG>`, :prop_tgt:`ARCHIVE_OUTPUT_NAME_<CONFIG>`,
:prop_tgt:`LIBRARY_OUTPUT_NAME_<CONFIG>` and
:prop_tgt:`RUNTIME_OUTPUT_NAME_<CONFIG>`, and the :prop_tgt:`<CONFIG>_POSTFIX`
and :prop_tgt:`DEBUG_POSTFIX` target properties.

Note that ``tgt`` is not added as a dependency of the target this expression is
evaluated on.
