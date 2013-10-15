VISIBILITY_INLINES_HIDDEN
-------------------------

Whether to add a compile flag to hide symbols of inline functions

The VISIBILITY_INLINES_HIDDEN property determines whether a flag for
hiding symbols for inline functions.  the value passed used in a
visibility related compile option, such as -fvisibility=.  This
property only has an affect for libraries and executables with
exports.  This property is initialized by the value of the variable
CMAKE_VISIBILITY_INLINES_HIDDEN if it is set when a target is created.
