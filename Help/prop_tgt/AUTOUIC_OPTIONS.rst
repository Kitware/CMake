AUTOUIC_OPTIONS
---------------

Additional options for uic when using autouic (see the :prop_tgt:`AUTOUIC` target property)

This property holds additional command line options
which will be used when uic is executed during the build via autouic,
i.e. it is equivalent to the optional OPTIONS argument of the
qt4_wrap_ui() macro.

By default it is empty.

This property is initialized by the value of the variable
:variable:`CMAKE_AUTOUIC` if it is set when a target is created.

The options set on the target may be overridden by :prop_sf:`AUTOUIC_OPTIONS` set
on the .ui source file.
