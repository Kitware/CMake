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
  "${RunCMake_BINARY_DIR}/generate_feature_list-build/cxx_features.txt"
  CXX_FEATURES
)

if (NOT CXX_FEATURES)
  run_cmake(NoSupportedCxxFeatures)
  run_cmake(NoSupportedCxxFeaturesGenex)
endif()

foreach(standard 98 11)
  file(READ
    "${RunCMake_BINARY_DIR}/generate_feature_list-build/cxx${standard}_flag.txt"
    CXX${standard}_FLAG
  )
  if (CXX${standard}_FLAG STREQUAL NOTFOUND)
    run_cmake(RequireCXX${standard})
    run_cmake(RequireCXX${standard}Variable)
  endif()
  if (CXX${standard}EXT_FLAG STREQUAL NOTFOUND)
    run_cmake(RequireCXX${standard}Ext)
    run_cmake(RequireCXX${standard}ExtVariable)
  endif()
endforeach()
