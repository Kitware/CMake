XCODE_SCHEME_WORKING_DIRECTORY
------------------------------

.. versionadded:: 3.17

Specify the ``Working Directory`` of the *Run* and *Profile*
actions in the generated Xcode scheme. In case the value contains
generator expressions those are evaluated.

This property is initialized by the value of the variable
:variable:`CMAKE_XCODE_SCHEME_WORKING_DIRECTORY` if it is set
when a target is created.

.. include:: ../include/XCODE_SCHEME_NOTE.rst

See also :prop_tgt:`DEBUGGER_WORKING_DIRECTORY`.
