CMAKE_CXX_KNOWN_FEATURES
------------------------

List of C++ features known to this version of CMake.

The features listed in this variable may be known to be available to the C++
compiler.  If the feature is available with the C++ compiler, it will be
listed in the :variable:`CMAKE_CXX_COMPILE_FEATURES` variable.

The features known to this version of CMake are:

cxx_delegating_constructors
  Delegating constructors, as defined in N1986_.

.. _N1986: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2006/n1986.pdf

cxx_variadic_templates
  Variadic templates, as defined in N2242_.

.. _N2242: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2242.pdf

gnuxx_typeof
  The GNU typeof extension.

msvcxx_sealed
  The MSVC sealed extension.
