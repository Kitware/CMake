target_compiler_features
------------------------

Add expected compiler features to a target.

::

  target_compiler_features(<target> PRIVATE <feature> [...])

Specify compiler features required when compiling a given target.  If the
feature is not listed in the :variable:`CMAKE_CXX_COMPILER_FEATURES` variable,
then an error will be reported by CMake.  If the use of the feature requires
an additional compiler flag, such as --std=c++11, the flag will be added
automatically.

The named <target> must have been created by a command such as
add_executable or add_library and must not be an IMPORTED target.
