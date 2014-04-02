CMAKE_CXX_KNOWN_FEATURES
------------------------

List of C++ features known to this version of CMake.

The features listed in this variable may be known to be available to the
C++ compiler.  If the feature is available with the C++ compiler, it will
be listed in the :variable:`CMAKE_CXX_COMPILE_FEATURES` variable.

The features listed here may be used with the :command:`target_compile_features`
command.

The features known to this version of CMake are:

``cxx_auto_type``
  Automatic type deduction, as defined in N1984_.

  .. _N1984: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2006/n1984.pdf

``cxx_constexpr``
  Constant expressions, as defined in N2235_.

  .. _N2235: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2235.pdf

``cxx_delegating_constructors``
  Delegating constructors, as defined in N1986_.

  .. _N1986: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2006/n1986.pdf

``cxx_final``
  Override control ``final`` keyword, as defined in N2928_.

  .. _N2928: http://www.open-std.org/JTC1/SC22/WG21/docs/papers/2009/n2928.htm

``cxx_override``
  Override control ``override`` keyword, as defined in N2928_.

  .. _N2928: http://www.open-std.org/JTC1/SC22/WG21/docs/papers/2009/n2928.htm

``cxx_static_assert``
  Static assert, as defined in N1720_.

  .. _N1720: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2004/n1720.html

``cxx_strong_enums``
  Strongly typed enums, as defined in N2347_.

  .. _N2347: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2347.pdf

``cxx_trailing_return_types``
  Automatic function return type, as defined in N2541_.

  .. _N2541: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2008/n2541.htm

``cxx_variadic_templates``
  Variadic templates, as defined in N2242_.

  .. _N2242: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2242.pdf
