INTERFACE_LINK_LIBRARIES
------------------------

List public interface libraries for a library.

This property contains the list of transitive link dependencies.  When
the target is linked into another target the libraries listed (and
recursively their link interface libraries) will be provided to the
other target also.  This property is overridden by the
LINK_INTERFACE_LIBRARIES or LINK_INTERFACE_LIBRARIES_<CONFIG> property
if policy CMP0022 is OLD or unset.

This property also supports generator expressions.  See the
:manual:`cmake-generator-expressions(7)` manual for available expressions.
