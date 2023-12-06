include(RunCMake)

# Test expects to be given a LIST of toolchain directories where a TIClang
# compiler binary is expected to be found relative to the "bin" directory:
# "-DCMake_TEST_TICLANG_TOOLCHAINS=<path1>;<path2>;<path3>"
if(RunCMake_GENERATOR MATCHES "Makefile|Ninja")
  set(_ticlang_toolchains "${CMake_TEST_TICLANG_TOOLCHAINS}" )
endif()

function(run_toolchain case)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${case}-build)
  run_cmake_with_options(${case} ${ARGN})
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${case}-build ${CMAKE_COMMAND} --build .)
endfunction()

foreach(_ticlang_toolchain_path IN LISTS _ticlang_toolchains)
  file(GLOB _ticlang_toolchain "${_ticlang_toolchain_path}/bin/*clang" )
  if(_ticlang_toolchain STREQUAL "")
    message(WARNING
          "Could not find a TIClang toolchain at: ${_ticlang_toolchain_path}.")
    continue()
  endif()

  message(STATUS "Found TIClang toolchain: ${_ticlang_toolchain}")

  if(_ticlang_toolchain MATCHES "tiarmclang")
    set(LINK_OPTS "--use_memcpy=fast,--use_memset=fast,-llnk.cmd")
    set(CMAKE_FLAGS "-mcpu=cortex-r5 -Oz")
  else()
    set(CMAKE_FLAGS "")
    set(LINK_OPTS "")
  endif()

  run_toolchain(ticlang-c
    -DCMAKE_SYSTEM_NAME=Generic
    -DCMAKE_C_COMPILER=${_ticlang_toolchain}
    -DCMAKE_C_FLAGS=${CMAKE_FLAGS}
    -DCMAKE_C_LINKER_FLAGS=${LINK_OPTS}
  )

  run_toolchain(ticlang-cxx
    -DCMAKE_SYSTEM_NAME=Generic
    -DCMAKE_CXX_COMPILER=${_ticlang_toolchain}
    -DCMAKE_CXX_FLAGS=${CMAKE_FLAGS}
    -DCMAKE_CXX_LINKER_FLAGS=${LINK_OPTS}
  )

  run_toolchain(ticlang-asm
    -DCMAKE_SYSTEM_NAME=Generic
    -DCMAKE_ASM_COMPILER=${_ticlang_toolchain}
    -DCMAKE_ASM_FLAGS=${CMAKE_FLAGS}
  )

  run_toolchain(ticlang-lib
    -DCMAKE_SYSTEM_NAME=Generic
    -DCMAKE_C_COMPILER=${_ticlang_toolchain}
    -DCMAKE_C_FLAGS=${CMAKE_FLAGS}
    -DCMAKE_C_LINKER_FLAGS=${LINK_OPTS}
  )
endforeach()
