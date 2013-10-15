<LANG>_VISIBILITY_PRESET
------------------------

Value for symbol visibility compile flags

The <LANG>_VISIBILITY_PRESET property determines the value passed in a
visibility related compile option, such as -fvisibility= for <LANG>.
This property only has an affect for libraries and executables with
exports.  This property is initialized by the value of the variable
CMAKE_<LANG>_VISIBILITY_PRESET if it is set when a target is created.
