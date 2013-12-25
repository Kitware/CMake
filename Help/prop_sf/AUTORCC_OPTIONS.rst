AUTORCC_OPTIONS
---------------

Additional options for rcc when using autorcc (see the :prop_tgt:`AUTORCC` target
property)

This property holds additional command line options which will be used when
rcc is executed during the build via autorcc, i.e. it is equivalent to the
optional OPTIONS argument of the qt4_add_resources() macro.

By default it is empty.

The options set on the .qrc source file may override :prop_tgt:`AUTORCC_OPTIONS` set
on the target.
