include(RunCMake)

run_cmake(CMP0037-OLD-space)
run_cmake(CMP0037-NEW-space)
run_cmake(CMP0037-WARN-space)
run_cmake(CMP0037-NEW-colon)

if(NOT "${RunCMake_GENERATOR}" MATCHES
    "((MSYS|MinGW|NMake|Borland) Makefiles|Watcom WMake)")
  run_cmake(CMP0037-WARN-colon)
endif()
