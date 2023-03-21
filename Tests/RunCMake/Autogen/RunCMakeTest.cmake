include(RunCMake)

run_cmake(NoQt)
if (DEFINED with_qt_version)
  set(RunCMake_TEST_OPTIONS
    -Dwith_qt_version=${with_qt_version}
    "-DQt${with_qt_version}_DIR:PATH=${Qt${with_qt_version}_DIR}"
    "-DCMAKE_PREFIX_PATH:STRING=${CMAKE_PREFIX_PATH}"
  )

  run_cmake(QtInFunction)
  run_cmake(QtInFunctionNested)
  run_cmake(QtInFunctionProperty)

  run_cmake(CMP0111-imported-target-full)
  run_cmake(CMP0111-imported-target-libname)
  run_cmake(CMP0111-imported-target-implib-only)

  block()
    set(RunCMake_TEST_BINARY_DIR  ${RunCMake_BINARY_DIR}/MocPredefs-build)
    run_cmake(MocPredefs)
    set(RunCMake_TEST_NO_CLEAN 1)
    run_cmake_command(MocPredefs-build ${CMAKE_COMMAND} --build . --config Debug)
  endblock()
endif ()
