include(RunCMake)

run_cmake_script(MAKE_DIRECTORY-one-dir-FAIL)
run_cmake_script(MAKE_DIRECTORY-Result-one-dir-FAIL)
run_cmake_script(MAKE_DIRECTORY-Result-one-dir-SUCCESS)
run_cmake_script(MAKE_DIRECTORY-Result-many-dirs-FAIL)
run_cmake_script(MAKE_DIRECTORY-Result-many-dirs-SUCCESS)
