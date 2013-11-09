
if(CMAKE_COMPILER_VERSION VERSION_LESS 2.1)
  # This file is only loaded if Clang >= 2.1
  message(FATAL_ERROR "This file should not be included for Clang < 2.1.")
endif()

set(testable_features
  cxx_delegating_constructors
)
foreach(feature ${testable_features})
  set(_cmake_feature_test_${feature} "__has_extension(${feature})")
endforeach()

unset(testable_features)

set(_cmake_feature_test_gnuxx_typeof "!defined(__STRICT_ANSI__)")
