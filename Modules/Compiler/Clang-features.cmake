
set(compiler_test_macro __clang__)

set(testable_features
  cxx_delegating_constructors
)
foreach(feature ${testable_features})
  set(feature_test_${feature} "__has_extension(${feature})")
endforeach()
