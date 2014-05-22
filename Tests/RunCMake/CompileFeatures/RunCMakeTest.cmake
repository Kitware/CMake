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
  "${RunCMake_BINARY_DIR}/generate_feature_list-build/c_features.txt"
  C_FEATURES
)
file(READ
  "${RunCMake_BINARY_DIR}/generate_feature_list-build/cxx_features.txt"
  CXX_FEATURES
)

if (NOT C_FEATURES)
  run_cmake(NoSupportedCFeatures)
  run_cmake(NoSupportedCFeaturesGenex)
endif()

if (NOT CXX_FEATURES)
  run_cmake(NoSupportedCxxFeatures)
  run_cmake(NoSupportedCxxFeaturesGenex)
else()
  run_cmake(LinkImplementationFeatureCycle)
  run_cmake(LinkImplementationFeatureCycleSolved)

  if (";${CXX_FEATURES};" MATCHES ";cxx_final;")
    set(RunCMake_TEST_OPTIONS "-DHAVE_FINAL=1")
  endif()
  run_cmake(NonValidTarget1)
  run_cmake(NonValidTarget2)
  unset(RunCMake_TEST_OPTIONS)
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
