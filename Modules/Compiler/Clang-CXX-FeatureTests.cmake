
set(testable_features
  cxx_delegating_constructors
  cxx_variadic_templates
)
foreach(feature ${testable_features})
  set(_cmake_feature_test_${feature} "__has_extension(${feature})")
endforeach()

unset(testable_features)

set(_cmake_feature_test_gnuxx_typeof "!defined(__STRICT_ANSI__)")
