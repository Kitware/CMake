AUTOGEN_USE_SYSTEM_INCLUDE
--------------------------

``AUTOGEN_USE_SYSTEM_INCLUDE`` is a boolean property that can be set
on a target to indicate that the autogen target include directory should
be added as a system include directory or normal include directory to the
target.

See the :manual:`cmake-qt(7)` manual for more information on using CMake
with Qt.

This property is initialized by the
:variable:`CMAKE_AUTOGEN_USE_SYSTEM_INCLUDE` variable if it is set when
a target is created.
