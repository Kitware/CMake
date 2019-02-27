include(RunCMake)
include(RunCTest)

run_cmake(Project)
file(WRITE "${RunCMake_BINARY_DIR}/Project-build/test.cmake" "# b")
run_cmake_command(ProjectBuild "${CMAKE_COMMAND}" --build "${RunCMake_BINARY_DIR}/Project-build")

run_cmake_command(Script "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_LIST_DIR}/Script.cmake")
run_cmake_command(FindPackage "${CMAKE_COMMAND}" --find-package -DNAME=DummyPackage -DCOMPILER_ID=GNU -DLANGUAGE=CXX -DMODE=EXIST "-DCMAKE_MODULE_PATH=${CMAKE_CURRENT_LIST_DIR}")
run_ctest(CTest)
run_cmake_command(BuildAndTest "${CMAKE_CTEST_COMMAND}"
  --build-and-test
      "${RunCMake_SOURCE_DIR}/BuildAndTest"
      "${RunCMake_BINARY_DIR}/BuildAndTest-build"
  --build-project CMakeRoleGlobalPropertyBuildAndTest
  --build-generator "${RunCMake_GENERATOR}"
  )
