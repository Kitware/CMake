POSITION_INDEPENDENT_CODE
-------------------------

Whether to create a position-independent target

The POSITION_INDEPENDENT_CODE property determines whether position
independent executables or shared libraries will be created.  This
property is true by default for SHARED and MODULE library targets and
false otherwise.  This property is initialized by the value of the
variable CMAKE_POSITION_INDEPENDENT_CODE if it is set when a target is
created.
