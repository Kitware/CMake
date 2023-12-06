set(var "CMAKE_Fortran_COMPILE_OPTIONS_MSVC_DEBUG_INFORMATION_FORMAT_Embedded")
string(REPLACE "-Z7" "-Z7;-DTEST_Z7" "${var}" "${${var}}")
set(var "CMAKE_Fortran_COMPILE_OPTIONS_MSVC_DEBUG_INFORMATION_FORMAT_ProgramDatabase")
string(REPLACE "-Zi" "-Zi;-DTEST_Zi" "${var}" "${${var}}")

if(CMAKE_Fortran_COMPILER_ID STREQUAL "LLVMFlang")
  set(var "CMAKE_Fortran_COMPILE_OPTIONS_MSVC_DEBUG_INFORMATION_FORMAT_Embedded")
  string(REPLACE "-g" "-g;-DTEST_Z7" "${var}" "${${var}}")
  # LLVMFlang does not actually support these, but Windows-LLVMFlang-Fortran pretends it does.
  set(CMAKE_Fortran_COMPILE_OPTIONS_MSVC_DEBUG_INFORMATION_FORMAT_ProgramDatabase "-DTEST_Zi")
  set(CMAKE_Fortran_COMPILE_OPTIONS_MSVC_DEBUG_INFORMATION_FORMAT_EditAndContinue "-DTEST_ZI")
endif()
