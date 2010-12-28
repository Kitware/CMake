if(NOT DEFINED source_dir)
  message(FATAL_ERROR "variable 'source_dir' not defined")
endif()

if(NOT EXISTS "${source_dir}/CMakeLists.txt")
  message(FATAL_ERROR "error: No CMakeLists.txt file to patch!")
endif()

set(text "

#
# Reference variables typically given as experimental_build_test configure
# options to avoid CMake warnings about unused variables
#

MESSAGE(\"Trilinos_ALLOW_NO_PACKAGES='\${Trilinos_ALLOW_NO_PACKAGES}'\")
MESSAGE(\"Trilinos_WARNINGS_AS_ERRORS_FLAGS='\${Trilinos_WARNINGS_AS_ERRORS_FLAGS}'\")
")

file(APPEND "${source_dir}/CMakeLists.txt" "${text}")
