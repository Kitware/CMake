include(RunCMake)

# Locate Diab toolchain
if(RunCMake_GENERATOR MATCHES "Makefile|Ninja")
  file(GLOB _diab_toolchains
    "${CMake_TEST_Diab_TOOLCHAINS}/*/*/bin/dcc*" )
  if(_diab_toolchains STREQUAL "")
    message(FATAL_ERROR "Could not find any Diab toolchains at: ${CMake_TEST_Diab_TOOLCHAINS}.")
  endif()
endif()

function(run_toolchain case)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${case}-build)
  run_cmake_with_options(${case} ${ARGN})
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${case}-build ${CMAKE_COMMAND} --build .)
endfunction()


foreach(_diab_toolchain IN LISTS _diab_toolchains)
  message(STATUS "Found Diab toolchain: ${_diab_toolchain}")
  cmake_path(GET _diab_toolchain PARENT_PATH BIN_DIR)
  set(c_comp ${BIN_DIR}/dcc)
  set(cxx_comp ${BIN_DIR}/dplus)

  # Create an executable from .c sources only.
  run_toolchain(diab-c
    -DCMAKE_C_COMPILER=${c_comp}
  )

  # Create an executale from .c and .cxx sources.
  run_toolchain(diab-cxx
    -DCMAKE_C_COMPILER=${c_comp}
    -DCMAKE_CXX_COMPILER=${cxx_comp}
  )

  # Create an executable from mixed, c, cxx and assembly.
  run_toolchain(diab-asm
    -DCMAKE_C_COMPILER=${c_comp}
    -DCMAKE_CXX_COMPILER=${cxx_comp}
  )
endforeach()
