include(RunCMake)

if(RunCMake_GENERATOR MATCHES "Makefile|Ninja")
  file(GLOB _renesas_toolchains
    "${CMake_TEST_Renesas_TOOLCHAINS}/*/*/bin/ccr*.exe" )
  if(_renesas_toolchains STREQUAL "")
    message(FATAL_ERROR "Could not find any Renesas toolchains at: ${CMake_TEST_Renesas_TOOLCHAINS}.")
  endif()
endif()

function(run_toolchain case)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${case}-build)
  run_cmake_with_options(${case} ${ARGN})
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${case}-build ${CMAKE_COMMAND} --build .)
endfunction()

foreach(_renesas_toolchain IN LISTS _renesas_toolchains)
  cmake_path(GET _renesas_toolchain PARENT_PATH BIN_DIR)
  cmake_path(GET _renesas_toolchain FILENAME BIN_NAME)

  if(BIN_NAME MATCHES "ccrx.exe")
# CC-RX
    set(ENV{BIN_RX} ${BIN_DIR})
    run_toolchain(renesas-c
      -DCMAKE_SYSTEM_NAME=Generic
      -DCMAKE_C_COMPILER=${_renesas_toolchain}
      -DCMAKE_C_FLAGS="-isa=rxv2"
      -DCMAKE_EXE_LINKER_FLAGS="-lnkopt=-start=P,C,D/100,B/8000"
    )
    run_toolchain(renesas-asm-rx
      -DCMAKE_SYSTEM_NAME=Generic
      -DCMAKE_C_COMPILER=${_renesas_toolchain}
      -DCMAKE_ASM_COMPILER=${_renesas_toolchain}
      -DCMAKE_C_FLAGS="-isa=rxv2"
      -DCMAKE_ASM_FLAGS="-isa=rxv2"
    )
  elseif(BIN_NAME MATCHES "ccrl.exe")
# CC-RL
    run_toolchain(renesas-c
      -DCMAKE_SYSTEM_NAME=Generic
      -DCMAKE_C_COMPILER=${_renesas_toolchain}
      -DCMAKE_C_FLAGS="-cpu=S2"
      -DCMAKE_EXE_LINKER_FLAGS="-lnkopt=-auto_section_layout"
    )
    run_toolchain(renesas-asm-rl
      -DCMAKE_SYSTEM_NAME=Generic
      -DCMAKE_C_COMPILER=${_renesas_toolchain}
      -DCMAKE_ASM_COMPILER=${_renesas_toolchain}
      -DCMAKE_C_FLAGS="-cpu=S2"
      -DCMAKE_ASM_FLAGS="-cpu=S2"
    )
  elseif(BIN_NAME MATCHES "ccrh.exe")
# CC-RH
    run_toolchain(renesas-c
      -DCMAKE_SYSTEM_NAME=Generic
      -DCMAKE_C_COMPILER=${_renesas_toolchain}
      -DCMAKE_C_FLAGS="-Xcommon=rh850"
      -DCMAKE_EXE_LINKER_FLAGS="-lnkopt=-start=SU,SI,B_1,R_1,C_1"
    )
    run_toolchain(renesas-asm-rh
      -DCMAKE_SYSTEM_NAME=Generic
      -DCMAKE_C_COMPILER=${_renesas_toolchain}
      -DCMAKE_ASM_COMPILER=${_renesas_toolchain}
      -DCMAKE_C_FLAGS="-Xcommon=rh850"
      -DCMAKE_ASM_FLAGS="-Xcommon=rh850"
    )
  endif()
endforeach()
