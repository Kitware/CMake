include(RunCMake)

set(cmd ${CMAKE_COMMAND} ${CMAKE_CURRENT_LIST_DIR} -G ${RunCMake_GENERATOR})

foreach(strictness IN ITEMS STRICT PERMISSIVE BEST_EFFORT)
  run_cmake_command(TestStrictness-${strictness} ${cmd}
    -DRunCMake_TEST=TestStrictness -DSTRICTNESS=${strictness}
  )
endforeach()

run_cmake(TestEnv)
run_cmake(TestExtract)
run_cmake(TestMangle)
run_cmake(TestQuiet)
run_cmake(TestRequired)
run_cmake(TestReroot)
run_cmake(TestUninstalled)
run_cmake(TestVersion)
