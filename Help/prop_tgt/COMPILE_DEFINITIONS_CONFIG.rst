COMPILE_DEFINITIONS_<CONFIG>
----------------------------

Ignored.  See CMake Policy :policy:`CMP0043`.

Per-configuration preprocessor definitions on a target.

This is the configuration-specific version of :prop_tgt:`COMPILE_DEFINITIONS`
where ``<CONFIG>`` is an upper-case name (ex. ``COMPILE_DEFINITIONS_DEBUG``).

.. |genex_subject| replace:: ``COMPILE_DEFINITIONS_<CONFIG>``
.. include:: ../include/GENEX_NOTE.rst

Generator expressions should be preferred instead of setting this property.
