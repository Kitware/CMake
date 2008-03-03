# This module defines macros intended for use by cross-compiling
# toolchain files when CMake is not able to automatically detect the
# compiler identification.
#
# Macro CMAKE_FORCE_C_COMPILER has the following signature:
#   CMAKE_FORCE_C_COMPILER(<compiler> <compiler-id>)
# It sets CMAKE_C_COMPILER to the given compiler and the cmake
# internal variable CMAKE_C_COMPILER_ID to the given compiler-id.
# It also bypasses the check for working compiler and basic compiler
# information tests.
#
# Macro CMAKE_FORCE_CXX_COMPILER has the following signature:
#   CMAKE_FORCE_CXX_COMPILER(<compiler> <compiler-id>)
# It sets CMAKE_CXX_COMPILER to the given compiler and the cmake
# internal variable CMAKE_CXX_COMPILER_ID to the given compiler-id.
# It also bypasses the check for working compiler and basic compiler
# information tests.
#
# So a simple toolchain file could look like this:
#   INCLUDE (CMakeForceCompiler)
#   SET(CMAKE_SYSTEM_NAME Generic)
#   CMAKE_FORCE_C_COMPILER   (chc12 MetrowerksHicross)
#   CMAKE_FORCE_CXX_COMPILER (chc12 MetrowerksHicross)

MACRO(CMAKE_FORCE_C_COMPILER compiler id)
  SET(CMAKE_C_COMPILER "${compiler}")
  SET(CMAKE_C_COMPILER_ID_RUN TRUE)
  SET(CMAKE_C_COMPILER_ID ${id})
  SET(CMAKE_C_COMPILER_WORKS TRUE)
  SET(CMAKE_C_COMPILER_FORCED TRUE)

  # Set old compiler id variables.
  IF("${CMAKE_C_COMPILER_ID}" MATCHES "GNU")
    SET(CMAKE_COMPILER_IS_GNUCC 1)
  ENDIF("${CMAKE_C_COMPILER_ID}" MATCHES "GNU")
ENDMACRO(CMAKE_FORCE_C_COMPILER)

MACRO(CMAKE_FORCE_CXX_COMPILER compiler id)
  SET(CMAKE_CXX_COMPILER "${compiler}")
  SET(CMAKE_CXX_COMPILER_ID_RUN TRUE)
  SET(CMAKE_CXX_COMPILER_ID ${id})
  SET(CMAKE_CXX_COMPILER_WORKS TRUE)
  SET(CMAKE_CXX_COMPILER_FORCED TRUE)

  # Set old compiler id variables.
  IF("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    SET(CMAKE_COMPILER_IS_GNUCXX 1)
  ENDIF("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
ENDMACRO(CMAKE_FORCE_CXX_COMPILER)
