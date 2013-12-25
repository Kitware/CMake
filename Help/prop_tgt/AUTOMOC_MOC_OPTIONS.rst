AUTOMOC_MOC_OPTIONS
-------------------

Additional options for moc when using automoc (see the AUTOMOC property)

This property is only used if the AUTOMOC property is set to TRUE for
this target.  In this case, it holds additional command line options
which will be used when moc is executed during the build, i.e.  it is
equivalent to the optional OPTIONS argument of the qt4_wrap_cpp()
macro.

By default it is empty.
