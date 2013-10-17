CMAKE_CXX_KNOWN_FEATURES
------------------------

List of C++ features known to this version of CMake.

The features listed in this variable may be known to be available to the C++
compiler.  If the feature is available with the C++ compiler, it will be
listed in the :variable:`CMAKE_CXX_COMPILER_FEATURES` variable.

The features known to this version of CMake are:

cxx_delegating_constructors
  Delegating constructors, as defined in N1986_.

.. N1986_: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2006/n1986.pdf
