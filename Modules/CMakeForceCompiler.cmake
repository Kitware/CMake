MESSAGE(FATAL_ERROR
  "Do not include the CMakeForceCompiler module.  "
  "It is no longer necessary.  "
  "Update your toolchain file as follows.

Use of the CMAKE_FORCE_SYSTEM macro:

  CMAKE_FORCE_SYSTEM(\"<name>\" \"<version>\" \"<processor>\")

may be replaced by just

  SET(CMAKE_SYSTEM_NAME \"<name>\")
  SET(CMAKE_SYSTEM_VERSION \"<version>\")
  SET(CMAKE_SYSTEM_PROCESSOR \"<processor>\")

Use of the CMAKE_FORCE_C_COMPILER and CMAKE_FORCE_CXX_COMPILER macros:

  CMAKE_FORCE_C_COMPILER   (/path/to/cc <id> <sizeof_void_p>)
  CMAKE_FORCE_CXX_COMPILER (/path/to/CC <id>)

may be replaced by just

  SET(CMAKE_C_COMPILER /path/to/cc)
  SET(CMAKE_CXX_COMPILER /path/to/CC)

CMake will automatically detect known compiler IDs and sizeof(void*).
")
