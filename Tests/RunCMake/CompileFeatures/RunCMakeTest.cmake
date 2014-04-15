include(RunCMake)

run_cmake(NotAFeature)
run_cmake(NotAFeatureGenex)
run_cmake(NotAFeatureTransitive)
run_cmake(NotAFeature_OriginDebug)
run_cmake(NotAFeature_OriginDebugGenex)
run_cmake(NotAFeature_OriginDebugTransitive)
run_cmake(NotAFeature_OriginDebug_target_compile_features)

run_cmake(generate_feature_list)
file(READ
  "${RunCMake_BINARY_DIR}/generate_feature_list-build/features.txt"
  FEATURES
)

if (NOT FEATURES)
  run_cmake(NoSupportedCxxFeatures)
  run_cmake(NoSupportedCxxFeaturesGenex)
endif()
