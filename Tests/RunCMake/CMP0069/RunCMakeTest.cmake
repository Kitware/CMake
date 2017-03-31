include(RunCMake)

run_cmake(CMP0069-OLD)
run_cmake(CMP0069-NEW-cmake)
run_cmake(CMP0069-NEW-compiler)
run_cmake(CMP0069-WARN)

string(COMPARE EQUAL "${RunCMake_GENERATOR}" "Xcode" is_xcode)
if(is_xcode OR RunCMake_GENERATOR MATCHES "^Visual Studio ")
  run_cmake(CMP0069-NEW-generator)
endif()
