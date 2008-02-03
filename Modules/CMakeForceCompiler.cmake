
# These are macros intended to be used only when crosscompiling in the 
# toolchain-file and only if the compiler is not able to link an 
# executable by default (usually because they need user-specific 
# linker files which describe the layout of the target memory).
#
# It offers the following macros:
#
# macro CMAKE_FORCE_SYSTEM(name version processor)
#   Set CMAKE_SYSTEM_NAME, CMAKE_SYSTEM_VERSION and CMAKE_SYSTEM_PROCESSOR
#
# macro CMAKE_FORCE_C_COMPILER(compiler compiler_id sizeof_void_p)
#   Set CMAKE_C_COMPILER to the given compiler and set CMAKE_C_COMPILER_ID
#   to the given compiler_id. This Id is used by cmake to construct the filename
#   of the system-compiler.cmake file. For C also the size of a void-pointer
#   has to be predefined.
#
# macro CMAKE_FORCE_CXX_COMPILER(compiler compiler_id)
#   The same as CMAKE_FORCE_C_COMPILER, but for CXX. Here the size of 
#   the void pointer is not requried.
#
# So a simple toolchain file could look like this:
#
# INCLUDE (CMakeForceCompiler)
# CMAKE_FORCE_SYSTEM ("Generic"   "0.0"   "hc12")
# CMAKE_FORCE_C_COMPILER   (chc12 FreescaleCHC12  2)
# CMAKE_FORCE_CXX_COMPILER (chc12 FreescaleCHC12)


MACRO(CMAKE_FORCE_SYSTEM name version proc)
   SET(CMAKE_SYSTEM_NAME "${name}")
   SET(CMAKE_SYSTEM_VERSION "${version}")
   SET(CMAKE_SYSTEM_PROCESSOR "${proc}")
ENDMACRO(CMAKE_FORCE_SYSTEM)

MACRO(CMAKE_FORCE_C_COMPILER compiler id sizeof_void)
  SET(CMAKE_C_COMPILER "${compiler}")
  SET(CMAKE_C_COMPILER_ID_RUN TRUE)
  SET(CMAKE_C_COMPILER_ID ${id})
  SET(CMAKE_C_COMPILER_WORKS TRUE)
  SET(CMAKE_C_COMPILER_FORCED TRUE)

  # Set old compiler and platform id variables.
  IF("${CMAKE_C_COMPILER_ID}" MATCHES "GNU")
    SET(CMAKE_COMPILER_IS_GNUCC 1)
  ENDIF("${CMAKE_C_COMPILER_ID}" MATCHES "GNU")

  SET(CMAKE_C_SIZEOF_DATA_PTR ${sizeof_void})
ENDMACRO(CMAKE_FORCE_C_COMPILER)

MACRO(CMAKE_FORCE_CXX_COMPILER compiler id)
  SET(CMAKE_CXX_COMPILER "${compiler}")
  SET(CMAKE_CXX_COMPILER_ID_RUN TRUE)
  SET(CMAKE_CXX_COMPILER_ID ${id})
  SET(CMAKE_CXX_COMPILER_WORKS TRUE)
  SET(CMAKE_CXX_COMPILER_FORCED TRUE)

  IF("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    SET(CMAKE_COMPILER_IS_GNUCXX 1)
  ENDIF("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")

ENDMACRO(CMAKE_FORCE_CXX_COMPILER)

