
set(_cmake_feature_test_gnuxx_typeof "!defined(__STRICT_ANSI__)")

if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.3)
  # This file is only loaded if GCC >= 4.3
  message(FATAL_ERROR "This file should not be included for GCC < 4.3.")
endif()

# For GCC 4.7, we can test the standard __cplusplus macro, ...
# set(isCxx98Mode "__cplusplus >= 199711L")
# set(isCxx11Mode "__cplusplus >= 201103L")
# ... but before GCC 4.7, __cplusplus is always defined to 1, so check
# the old macro.
# set(isCxx98Mode "!defined(__GXX_EXPERIMENTAL_CXX0X__)")
# set(isCxx11Mode "defined(__GXX_EXPERIMENTAL_CXX0X__)")

set(_cmake_feature_test_cxx_delegating_constructors "(__GNUC__ * 100 + __GNUC_MINOR__) >= 407 && __cplusplus >= 201103L")
