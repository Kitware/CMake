CMAKE_POSITION_INDEPENDENT_CODE
-------------------------------

Default value for :prop_tgt:`POSITION_INDEPENDENT_CODE` of targets.

This variable is used to initialize the
:prop_tgt:`POSITION_INDEPENDENT_CODE` property on targets that
are not ``SHARED`` or ``MODULE`` library targets.
If set, its value is also used by the :command:`try_compile` command.
