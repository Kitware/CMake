
set(testable_features
  cxx_delegating_constructors
)
foreach(feature ${testable_features})
  set(_cmake_feature_test_${feature} "__has_extension(${feature})")
endforeach()

set(_cmake_feature_test_gnuxx_typeof "!defined(__STRICT_ANSI__)")
if (SIMULATED_COMPILER_ID STREQUAL MSVC)
  set(_cmake_feature_test_msvcxx_sealed "1")
endif()
