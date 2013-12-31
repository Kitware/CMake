AUTOUIC_OPTIONS
---------------

Additional options for uic when using autouic (see the :prop_tgt:`AUTOUIC` target property)

This property holds additional command line options
which will be used when uic is executed during the build via autouic,
i.e. it is equivalent to the optional OPTIONS argument of the
qt4_wrap_ui() macro.

By default it is empty.

The options set on the .ui source file may override :prop_tgt:`AUTOUIC_OPTIONS` set
on the target.
