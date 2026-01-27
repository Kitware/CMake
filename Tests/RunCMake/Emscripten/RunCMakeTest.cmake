include(RunCMake)

# Locate Emscripten toolchain
if(RunCMake_GENERATOR MATCHES "Makefile|Ninja")
  file(GLOB _emscripten_toolchains
    "${CMake_TEST_Emscripten_TOOLCHAINS}/emcc" )
  if(_emscripten_toolchains STREQUAL "")
    message(FATAL_ERROR "Could not find any Emscripten toolchains at: ${CMake_TEST_Emscripten_TOOLCHAINS}.")
  endif()
endif()

function(run_toolchain case)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${case}-build)
  run_cmake_with_options(${case} ${ARGN})
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OUTPUT_MERGE 1)
  run_cmake_command(${case}-build ${CMAKE_COMMAND} --build .)
endfunction()

macro(run_cmake_target test subtest target)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${test}-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OUTPUT_MERGE 1)
  run_cmake_command(${test}-${subtest}-build ${CMAKE_COMMAND} --build . --target ${target} --config Release --verbose ${ARGN})
  unset(RunCMake_TEST_BINARY_DIR)
  unset(RunCMake_TEST_NO_CLEAN)
endmacro()

foreach(_emscripten_toolchain IN LISTS _emscripten_toolchains)
  message(STATUS "Found Emscripten toolchain: ${_emscripten_toolchain}")
  cmake_path(GET _emscripten_toolchain PARENT_PATH BIN_DIR)

  if (WIN32)
    set(EMCC_SUFFIX ".bat")
  else()
    set(EMCC_SUFFIX "")
  endif()

  set(c_comp ${BIN_DIR}/emcc${EMCC_SUFFIX})
  set(cxx_comp ${BIN_DIR}/em++${EMCC_SUFFIX})
  set(comp_ar ${BIN_DIR}/emar${EMCC_SUFFIX})

  # Compiler inspection.
  run_cmake_with_options(C-enable
    -DCMAKE_SYSTEM_NAME=Emscripten
    -DCMAKE_C_COMPILER=${c_comp}
  )

  # Create an executable from .c sources only.
  run_toolchain(C-exe
    -DCMAKE_SYSTEM_NAME=Emscripten
    -DCMAKE_C_COMPILER=${c_comp}
  )

  # Create an executable from .c and .cxx sources.
  run_toolchain(CXX-exe
    -DCMAKE_SYSTEM_NAME=Emscripten
    -DCMAKE_C_COMPILER=${c_comp}
    -DCMAKE_CXX_COMPILER=${cxx_comp}
  )

  # Create a static library and executable from .c sources.
  run_toolchain(C-lib-static
    -DCMAKE_SYSTEM_NAME=Emscripten
    -DCMAKE_C_COMPILER=${c_comp}
  )

  # Create a shared library and executable from .c sources.
  run_toolchain(C-lib-shared
    -DCMAKE_SYSTEM_NAME=Emscripten
    -DCMAKE_C_COMPILER=${c_comp}
  )

  run_toolchain(C-lib-circular
    -DCMAKE_SYSTEM_NAME=Emscripten
    -DCMAKE_C_COMPILER=${c_comp}
  )

  run_cmake_with_options(C-WHOLE_ARCHIVE
    -DCMAKE_SYSTEM_NAME=Emscripten
    -DCMAKE_C_COMPILER=${c_comp}
  )
  run_cmake_target(C-WHOLE_ARCHIVE link-exe main)
  run_cmake_target(C-WHOLE_ARCHIVE circular-exe main_circular)

  run_cmake_with_options(C-CheckTypeSize
    -DCMAKE_SYSTEM_NAME=Emscripten
    -DCMAKE_C_COMPILER=${c_comp}
  )

  if(CMake_TEST_Emscripten_NODE)
    run_cmake_with_options(C-try_run
      -DCMAKE_SYSTEM_NAME=Emscripten
      -DCMAKE_CROSSCOMPILING_EMULATOR=${CMake_TEST_Emscripten_NODE}
      -DCMAKE_C_COMPILER=${c_comp}
    )
  endif()
endforeach()
