include(RunCMake)

run_cmake(NoQt)
if (with_qt5)
  run_cmake(QtInFunction)
  run_cmake(QtInFunctionNested)
  run_cmake(QtInFunctionProperty)

  run_cmake(CMP0111-imported-target-full)
  run_cmake(CMP0111-imported-target-libname)
  run_cmake(CMP0111-imported-target-implib-only)
endif ()
