DEBUGGER_WORKING_DIRECTORY
--------------------------

.. versionadded:: 4.0

Sets the local debugger working directory for C++ targets.
The property value may use
:manual:`generator expressions <cmake-generator-expressions(7)>`.
This property is initialized by the value of the variable
:variable:`CMAKE_DEBUGGER_WORKING_DIRECTORY` if it is set when a target is
created.
