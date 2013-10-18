include(RunCMake)

foreach(p
    )
  run_cmake(${p}-WARN)
  run_cmake(${p}-OLD)
  run_cmake(${p}-NEW)
endforeach()
