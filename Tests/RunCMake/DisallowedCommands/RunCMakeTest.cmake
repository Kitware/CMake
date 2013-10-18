include(RunCMake)

foreach(p
    CMP0029
    )
  run_cmake(${p}-WARN)
  run_cmake(${p}-OLD)
  run_cmake(${p}-NEW)
endforeach()
