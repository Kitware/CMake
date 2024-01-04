include(RunCMake)

run_cmake(ParseImplicitLinkInfo)

# Detect information from the toolchain:
# - CMAKE_SYSTEM_NAME
# - CMAKE_C_COMPILER
# - CMAKE_C_COMPILER_ID
# - CMAKE_C_COMPILER_VERSION
# - CMAKE_C_COMPILER_LINKER
# - CMAKE_C_COMPILER_LINKER_ID
# - CMAKE_C_COMPILER_LINKER_VERSION
# - CMAKE_C_IMPLICIT_LINK_DIRECTORIES
run_cmake(Inspect)
set(info "${RunCMake_BINARY_DIR}/Inspect-build/info.cmake")
include("${info}")

if(CMAKE_HOST_UNIX)
  run_cmake_script(DetermineLinkerId)
endif()

if(CMAKE_C_IMPLICIT_LINK_DIRECTORIES MATCHES ";")
  run_cmake_with_options(ExcludeDirs "-Dinfo=${RunCMake_BINARY_DIR}/Inspect-build/info.cmake")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "^(Linux|Darwin|Windows|AIX|SunOS)$|BSD"
    AND NOT CMAKE_C_COMPILER_ID MATCHES "^(Borland|Embarcadero|OpenWatcom|OrangeC|Watcom)$"
    AND NOT (CMAKE_C_COMPILER_ID MATCHES "^(Intel|IntelLLVM)$" AND CMAKE_SYSTEM_NAME STREQUAL "Windows")
    AND NOT CMAKE_C_COMPILER_LINKER MATCHES "Visual Studio 9\\.0"
    AND NOT RunCMake_GENERATOR MATCHES "Visual Studio 9 "
    )
  if(NOT CMAKE_C_COMPILER_LINKER OR NOT CMAKE_C_COMPILER_LINKER_ID OR NOT CMAKE_C_COMPILER_LINKER_VERSION)
    message(SEND_ERROR "C compiler's linker not identified:\n"
      "  CMAKE_C_COMPILER='${CMAKE_C_COMPILER}'\n"
      "  CMAKE_C_COMPILER_ID='${CMAKE_C_COMPILER_ID}'\n"
      "  CMAKE_C_COMPILER_VERSION='${CMAKE_C_COMPILER_VERSION}'\n"
      "  CMAKE_C_COMPILER_LINKER='${CMAKE_C_COMPILER_LINKER}'\n"
      "  CMAKE_C_COMPILER_LINKER_ID='${CMAKE_C_COMPILER_LINKER_ID}'\n"
      "  CMAKE_C_COMPILER_LINKER_VERSION='${CMAKE_C_COMPILER_LINKER_VERSION}'\n"
      )
  endif()
endif()
