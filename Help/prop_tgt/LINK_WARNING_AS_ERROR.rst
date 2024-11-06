LINK_WARNING_AS_ERROR
---------------------

.. versionadded:: 3.32

Specify whether to treat warnings on link as errors.
If enabled, adds a flag to treat warnings on link as errors.
If the :option:`cmake --link-no-warning-as-error` option is given
on the :manual:`cmake(1)` command line, this property is ignored.

This property is not implemented for all linkers.  It is silently ignored
if there is no implementation for the linker being used.  The currently
implemented :variable:`compiler linker IDs <CMAKE_<LANG>_COMPILER_LINKER_ID>`
are:

* ``AIX``
* ``AppleClang``
* ``GNU``
* ``GNUgold``
* ``LLD``
* ``MOLD``
* ``MSVC``
* ``Solaris``

This property is initialized by the value of the variable
:variable:`CMAKE_LINK_WARNING_AS_ERROR` if it is set when a target is
created.
