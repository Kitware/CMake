include(RunCMake)

if(RunCMake_GENERATOR MATCHES "Makefile|Ninja")
  file(GLOB _iar_toolchains
    "${CMake_TEST_IAR_TOOLCHAINS}/bx*-*/*/bin/icc*" )
  if(_iar_toolchains STREQUAL "")
    message(FATAL_ERROR "Could not find any IAR toolchains at: ${CMake_TEST_IAR_TOOLCHAINS}.")
  endif()
endif()

foreach(_iar_toolchain IN LISTS _iar_toolchains)
  message(STATUS "Found IAR toolchain: ${_iar_toolchain}")
  cmake_path(GET _iar_toolchain PARENT_PATH BIN_DIR)
  cmake_path(GET BIN_DIR PARENT_PATH TOOLKIT_DIR)
  cmake_path(GET TOOLKIT_DIR FILENAME ARCH)

  # Sets the minimal requirements for linking each target architecture
  if(ARCH STREQUAL rl78)
    set(LINK_OPTS
"--config_def _STACK_SIZE=256 \
--config_def _NEAR_HEAP_SIZE=0x400 \
--config_def _FAR_HEAP_SIZE=4096 \
--config_def _HUGE_HEAP_SIZE=0 \
--config_def _NEAR_CONST_LOCATION_START=0x2000 \
--config_def _NEAR_CONST_LOCATION_SIZE=0x6F00 \
--define_symbol _NEAR_CONST_LOCATION=0 \
--config ${TOOLKIT_DIR}/config/lnkrl78_s3.icf" )
  else()
    set(LINK_OPTS "")
  endif()

  # Set IAR Assembler (ILINK || XLINK)
  find_program(IAR_ASSEMBLER
    NAMES iasm${ARCH} a${ARCH}
    PATHS ${BIN_DIR}
    REQUIRED )

  set(RunCMake_TEST_OPTIONS
    -DCMAKE_SYSTEM_NAME=Generic
    -DCMAKE_C_COMPILER=${_iar_toolchain}
    -DCMAKE_EXE_LINKER_FLAGS=${LINK_OPTS}
  )
  run_cmake(iar-c)

  set(RunCMake_TEST_OPTIONS
    -DCMAKE_SYSTEM_NAME=Generic
    -DCMAKE_CXX_COMPILER=${_iar_toolchain}
    -DCMAKE_EXE_LINKER_FLAGS=${LINK_OPTS}
  )
  run_cmake(iar-cxx)

  set(RunCMake_TEST_OPTIONS
    -DCMAKE_SYSTEM_NAME=Generic
    -DCMAKE_ASM_COMPILER=${IAR_ASSEMBLER}
    )
  run_cmake(iar-asm)

  set(RunCMake_TEST_OPTIONS
    -DCMAKE_SYSTEM_NAME=Generic
    -DCMAKE_C_COMPILER=${_iar_toolchain}
    -DCMAKE_EXE_LINKER_FLAGS=${LINK_OPTS}
    )
  run_cmake(iar-lib)
endforeach()
