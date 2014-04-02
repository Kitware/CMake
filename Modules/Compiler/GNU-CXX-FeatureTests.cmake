
# Reference: http://gcc.gnu.org/projects/cxx0x.html

set(_oldestSupported "(__GNUC__ * 100 + __GNUC_MINOR__) >= 408")
# TODO: Should be supported by GNU 4.7
set(GNU47_CXX11 "${_oldestSupported} && __cplusplus >= 201103L")
set(_cmake_feature_test_cxx_delegating_constructors "${GNU47_CXX11}")
set(_cmake_feature_test_cxx_final "${GNU47_CXX11}")
set(_cmake_feature_test_cxx_override "${GNU47_CXX11}")
# NOTE: C++11 was ratified in September 2011. GNU 4.7 is the first minor
# release following that (March 2012), and the first minor release to
# support -std=c++11. Prior to that, support for C++11 features is technically
# experiemental and possibly incomplete (see for example the note below about
# cxx_variadic_template_template_parameters)
# TODO: Should be supported by GNU 4.6
set(GNU46_CXX11 "${_oldestSupported} && __cplusplus >= 201103L")
set(_cmake_feature_test_cxx_constexpr "${GNU46_CXX11}")
# TODO: Should be supported by GNU 4.4
set(GNU44_CXX11 "${_oldestSupported} && __cplusplus >= 201103L")
set(_cmake_feature_test_cxx_auto_type "${GNU44_CXX11}")
set(_cmake_feature_test_cxx_strong_enums "${GNU44_CXX11}")
set(_cmake_feature_test_cxx_trailing_return_types "${GNU44_CXX11}")
set(_cmake_feature_test_cxx_variadic_templates "${GNU44_CXX11}")
# TODO: If features are ever recorded for GNU 4.3, there should possibly
# be a new feature added like cxx_variadic_template_template_parameters,
# which is implemented by GNU 4.4, but not 4.3. cxx_variadic_templates is
# actually implemented by GNU 4.3, but variadic template template parameters
# 'completes' it, so that is the version we record as having the variadic
# templates capability in CMake. See
# http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2008/n2555.pdf
# TODO: Should be supported by GNU 4.3
set(GNU43_CXX11 "${_oldestSupported} && __cplusplus >= 201103L")
set(_cmake_feature_test_cxx_decltype "${GNU43_CXX11}")
set(_cmake_feature_test_cxx_static_assert "${GNU43_CXX11}")
set(_oldestSupported)
