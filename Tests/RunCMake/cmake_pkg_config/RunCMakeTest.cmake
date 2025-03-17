include(RunCMake)

set(cmd ${CMAKE_COMMAND} ${CMAKE_CURRENT_LIST_DIR} -G ${RunCMake_GENERATOR})

foreach(strictness IN ITEMS STRICT PERMISSIVE BEST_EFFORT)
  run_cmake_command(ExtractStrictness-${strictness} ${cmd}
    -DRunCMake_TEST=ExtractStrictness -DSTRICTNESS=${strictness}
  )
endforeach()

run_cmake(ExtractEnv)
run_cmake(ExtractFields)
run_cmake(ExtractMangle)
run_cmake(ExtractQuiet)
run_cmake(ExtractRequired)
run_cmake(ExtractReroot)
run_cmake(ExtractUninstalled)
run_cmake(ExtractVersion)
run_cmake(ImportName)
run_cmake(ImportPrefix)
run_cmake(ImportRequires)
run_cmake(ImportSimple)
run_cmake(ImportTransitiveFail)
run_cmake(ImportTransitiveVersion)
run_cmake(ImportTransitiveVersionFail)
run_cmake(PopulateFoundVar)
run_cmake(PopulateMissing)
