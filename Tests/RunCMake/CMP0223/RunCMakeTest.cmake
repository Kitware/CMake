include(RunCMake)

run_cmake(CMP0223-OLD)
run_cmake(CMP0223-WARN)
run_cmake(CMP0223-NEW)

run_cmake_with_options(CMP0223-genex-OLD -DCMAKE_POLICY_DEFAULT_CMP0223=OLD)
run_cmake_with_options(CMP0223-genex-NEW -DCMAKE_POLICY_DEFAULT_CMP0223=NEW)
run_cmake(CMP0223-genex-WARN)

run_cmake(CMP0223-if-WARN)
