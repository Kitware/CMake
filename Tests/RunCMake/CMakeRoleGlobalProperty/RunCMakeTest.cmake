include(RunCMake)
include(RunCTest)

run_cmake(Project)
run_cmake_command(Script "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_LIST_DIR}/Script.cmake")
run_cmake_command(FindPackage "${CMAKE_COMMAND}" --find-package -DNAME=DummyPackage -DCOMPILER_ID=GNU -DLANGUAGE=CXX -DMODE=EXIST "-DCMAKE_MODULE_PATH=${CMAKE_CURRENT_LIST_DIR}")
run_ctest(CTest)
