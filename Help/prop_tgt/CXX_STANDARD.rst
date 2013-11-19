CXX_STANDARD
------------

The C++ standard whose features are required to build this target.

This property specifies the C++ standard whose features are required
to build this target.  For some compilers, this results in adding a
flag such as -std=c++11 to the compile line.

Supported values are 98 and 11.

This property is initialized by the value of the :variable:`CMAKE_CXX_STANDARD`
variable if it is set when a target is created.
