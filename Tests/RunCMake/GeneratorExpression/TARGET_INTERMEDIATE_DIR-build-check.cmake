file(READ "${RunCMake_TEST_BINARY_DIR}/TARGET_INTERMEDIATE_DIR-generated.txt" dir)

# Default Case
set(expected "foo\\.dir")

# Short Object Names
if (CMAKE_INTERMEDIATE_DIR_STRATEGY STREQUAL "SHORT")
  if (RunCMake_GENERATOR MATCHES "Ninja|Make|Visual Studio")
    set(expected "[\\\\/][._]o[\\\\/][0-9a-f]+")
  endif()
endif()

# Xcode
if (RunCMake_GENERATOR MATCHES "Xcode")
  set(expected "foo.build")
endif()

# Append Config subdirectory
if (RunCMake_GENERATOR_IS_MULTI_CONFIG)
  string(APPEND expected "[\\\\/]Release")
endif()

# Xcode has additional paths
if (NOT RunCMake_GENERATOR MATCHES "Xcode")
  string(APPEND expected "$")
endif()

if(NOT dir MATCHES "${expected}")
  set(RunCMake_TEST_FAILED "actual content:\n [[${dir}]]\nbut expected to match:\n [[${expected}]]")
elseif(NOT IS_DIRECTORY "${dir}")
  set(RunCMake_TEST_FAILED "target intermediate directory does not exist: [[${dir}]]")
else()
  file(GLOB object_files "${dir}/*${CMAKE_C_OUTPUT_EXTENSION}")
  if (NOT object_files)
    set(RunCMake_TEST_FAILED "no object files found in intermediate directory: [[${dir}]]")
  endif()
  if (CMAKE_INTERMEDIATE_DIR_STRATEGY STREQUAL "FULL")
    set(object_name "${dir}/simple.c${CMAKE_C_OUTPUT_EXTENSION}")
    if (RunCMake_GENERATOR MATCHES "Xcode|Visual Studio")
      set(object_name "${dir}/simple${CMAKE_C_OUTPUT_EXTENSION}")
    endif()
    if (NOT EXISTS "${object_name}")
      set(RunCMake_TEST_FAILED "simple.c object file not found in intermediate directory: [[${dir}]]")
    endif()
  endif()
endif()
