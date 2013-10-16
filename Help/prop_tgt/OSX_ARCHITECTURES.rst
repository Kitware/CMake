OSX_ARCHITECTURES
-----------------

Target specific architectures for OS X.

The OSX_ARCHITECTURES property sets the target binary architecture for
targets on OS X.  This property is initialized by the value of the
variable CMAKE_OSX_ARCHITECTURES if it is set when a target is
created.  Use OSX_ARCHITECTURES_<CONFIG> to set the binary
architectures on a per-configuration basis.  <CONFIG> is an upper-case
name (ex: "OSX_ARCHITECTURES_DEBUG").
