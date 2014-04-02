
# Reference: http://gcc.gnu.org/projects/cxx0x.html

set(_oldestSupported "(__GNUC__ * 100 + __GNUC_MINOR__) >= 408")
# TODO: Should be supported by GNU 4.4
set(GNU44_CXX11 "${_oldestSupported} && __cplusplus >= 201103L")
set(_cmake_feature_test_cxx_auto_type "${GNU44_CXX11}")
set(_oldestSupported)
