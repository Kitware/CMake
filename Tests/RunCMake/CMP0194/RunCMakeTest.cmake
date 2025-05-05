include(RunCMake)

# The test cases empty the PATH before enabling ASM to avoid finding
# another assembler in the caller's environment.  However, old
# versions of MSVC do not support running `cl` without the PATH set.
if(NOT CMAKE_C_COMPILER_VERSION VERSION_LESS 16)
  run_cmake(CMP0194-WARN)
  run_cmake(CMP0194-OLD)
endif()
run_cmake(CMP0194-NEW)
