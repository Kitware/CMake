# try to load any previously computed information for C on this platform
include( ${CMAKE_PLATFORM_ROOT_BIN}/CMakeCPlatform.cmake OPTIONAL)
# try to load any previously computed information for CXX on this platform
include( ${CMAKE_PLATFORM_ROOT_BIN}/CMakeCXXPlatform.cmake OPTIONAL)

set(WIN32 1)

include(Platform/cl)

set(CMAKE_CREATE_WIN32_EXE /subsystem:windows)
set(CMAKE_CREATE_CONSOLE_EXE /subsystem:console)

if(CMAKE_GENERATOR MATCHES "Visual Studio 6")
   set(CMAKE_NO_BUILD_TYPE 1)
endif(CMAKE_GENERATOR MATCHES "Visual Studio 6")
if(NOT CMAKE_NO_BUILD_TYPE AND CMAKE_GENERATOR MATCHES "Visual Studio")
  set(CMAKE_NO_BUILD_TYPE 1)
  set(CMAKE_CONFIGURATION_TYPES "Debug;Release;MinSizeRel;RelWithDebInfo" CACHE STRING
     "Semicolon separated list of supported configuration types, only supports Debug, Release, MinSizeRel, and RelWithDebInfo, anything else will be ignored.")
  mark_as_advanced(CMAKE_CONFIGURATION_TYPES)
endif(NOT CMAKE_NO_BUILD_TYPE AND CMAKE_GENERATOR MATCHES "Visual Studio")
# does the compiler support pdbtype and is it the newer compiler
if(CMAKE_GENERATOR MATCHES  "Visual Studio 8")
  set(CMAKE_COMPILER_2005 1)
endif(CMAKE_GENERATOR MATCHES  "Visual Studio 8")

# make sure to enable languages after setting configuration types
enable_language(RC)
set(CMAKE_COMPILE_RESOURCE "rc <FLAGS> /fo<OBJECT> <SOURCE>")

# for nmake we need to compute some information about the compiler
# that is being used.
# the compiler may be free command line, 6, 7, or 71, and
# each have properties that must be determined.
# to avoid running these tests with each cmake run, the
# test results are saved in CMakeCPlatform.cmake, a file
# that is automatically copied into try_compile directories
# by the global generator.
set(MSVC_IDE 1)
if(CMAKE_GENERATOR MATCHES "Makefiles")
  set(MSVC_IDE 0)
  if(NOT CMAKE_VC_COMPILER_TESTS_RUN)
    set(CMAKE_VC_COMPILER_TESTS 1)
    set(testNmakeCLVersionFile
      "${CMAKE_ROOT}/Modules/CMakeTestNMakeCLVersion.c")
    string(REGEX REPLACE "/" "\\\\" testNmakeCLVersionFile "${testNmakeCLVersionFile}")
    message(STATUS "Check for CL compiler version")
    set(CMAKE_TEST_COMPILER ${CMAKE_C_COMPILER})
    if(NOT CMAKE_C_COMPILER)
      set(CMAKE_TEST_COMPILER ${CMAKE_CXX_COMPILER})
    endif(NOT CMAKE_C_COMPILER)
    exec_program(${CMAKE_TEST_COMPILER}
      ARGS /nologo -EP \"${testNmakeCLVersionFile}\"
      OUTPUT_VARIABLE CMAKE_COMPILER_OUTPUT
      RETURN_VALUE CMAKE_COMPILER_RETURN
      )
    if(NOT CMAKE_COMPILER_RETURN)
      file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
        "Determining the version of compiler passed with the following output:\n"
        "${CMAKE_COMPILER_OUTPUT}\n\n")
      string(REGEX REPLACE "\n" " " compilerVersion "${CMAKE_COMPILER_OUTPUT}")
      string(REGEX REPLACE ".*VERSION=(.*)" "\\1"
        compilerVersion "${compilerVersion}")
      message(STATUS "Check for CL compiler version - ${compilerVersion}")
      set(MSVC60)
      set(MSVC70)
      set(MSVC71)
      set(MSVC80)
      set(CMAKE_COMPILER_2005)
      if("${compilerVersion}" LESS 1300)
        set(MSVC60 1)
        set(CMAKE_COMPILER_SUPPORTS_PDBTYPE 1)
      endif("${compilerVersion}" LESS 1300)
      if("${compilerVersion}" EQUAL 1300)
        set(MSVC70 1)
        set(CMAKE_COMPILER_SUPPORTS_PDBTYPE 0)
      endif("${compilerVersion}" EQUAL 1300)
      if("${compilerVersion}" EQUAL 1310)
        set(MSVC71 1)
        set(CMAKE_COMPILER_SUPPORTS_PDBTYPE 0)
      endif("${compilerVersion}" EQUAL 1310)
      if("${compilerVersion}" EQUAL 1400)
        set(MSVC80 1)
        set(CMAKE_COMPILER_2005 1)
      endif("${compilerVersion}" EQUAL 1400)
      if("${compilerVersion}" EQUAL 1500)
        set(MSVC90 1)
      endif("${compilerVersion}" EQUAL 1500)
      if("${compilerVersion}" EQUAL 1600)
        set(MSVC10 1)
      endif("${compilerVersion}" EQUAL 1600)
      set(MSVC_VERSION "${compilerVersion}")
    else(NOT CMAKE_COMPILER_RETURN)
      message(STATUS "Check for CL compiler version - failed")
      file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
        "Determining the version of compiler failed with the following output:\n"
        "${CMAKE_COMPILER_OUTPUT}\n\n")
    endif(NOT CMAKE_COMPILER_RETURN)
    # try to figure out if we are running the free command line
    # tools from Microsoft.  These tools do not provide debug libraries,
    # so the link flags used have to be different.
    make_directory("${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp2")
    set(testForFreeVCFile
      "${CMAKE_ROOT}/Modules/CMakeTestForFreeVC.cxx")
    string(REGEX REPLACE "/" "\\\\" testForFreeVCFile "${testForFreeVCFile}")
    message(STATUS "Check if this is a free VC compiler")
    exec_program(${CMAKE_TEST_COMPILER} ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp2
      ARGS /nologo /MD /EHsc
      \"${testForFreeVCFile}\"
      OUTPUT_VARIABLE CMAKE_COMPILER_OUTPUT
      RETURN_VALUE CMAKE_COMPILER_RETURN
      )
    if(CMAKE_COMPILER_RETURN)
      file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
        "Determining if this is a free VC compiler failed with the following output:\n"
        "${CMAKE_COMPILER_OUTPUT}\n\n")
      message(STATUS "Check if this is a free VC compiler - yes")
      set(CMAKE_USING_VC_FREE_TOOLS 1)
    else(CMAKE_COMPILER_RETURN)
      file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
        "Determining if this is a free VC compiler passed with the following output:\n"
        "${CMAKE_COMPILER_OUTPUT}\n\n")
      message(STATUS "Check if this is a free VC compiler - no")
      set(CMAKE_USING_VC_FREE_TOOLS 0)
    endif(CMAKE_COMPILER_RETURN)
    make_directory("${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp3")
  endif(NOT CMAKE_VC_COMPILER_TESTS_RUN)
endif(CMAKE_GENERATOR MATCHES "Makefiles")

if(MSVC_C_ARCHITECTURE_ID MATCHES 64)
  set(CMAKE_CL_64 1)
else(MSVC_C_ARCHITECTURE_ID MATCHES 64)
  set(CMAKE_CL_64 0)
endif(MSVC_C_ARCHITECTURE_ID MATCHES 64)
if(CMAKE_FORCE_WIN64 OR CMAKE_FORCE_IA64)
  set(CMAKE_CL_64 1)
endif(CMAKE_FORCE_WIN64 OR CMAKE_FORCE_IA64)

if("${MSVC_VERSION}" GREATER 1599)
  set(MSVC_INCREMENTAL_DEFAULT ON)
endif()

# default to Debug builds
if(MSVC_VERSION GREATER 1310)
  # for 2005 make sure the manifest is put in the dll with mt
  set(CMAKE_CXX_CREATE_SHARED_LIBRARY "<CMAKE_COMMAND> -E vs_link_dll ${CMAKE_CXX_CREATE_SHARED_LIBRARY}")
  set(CMAKE_CXX_CREATE_SHARED_MODULE "<CMAKE_COMMAND> -E vs_link_dll ${CMAKE_CXX_CREATE_SHARED_MODULE}")
  # create a C shared library
  set(CMAKE_C_CREATE_SHARED_LIBRARY "${CMAKE_CXX_CREATE_SHARED_LIBRARY}")
  # create a C shared module just copy the shared library rule
  set(CMAKE_C_CREATE_SHARED_MODULE "${CMAKE_CXX_CREATE_SHARED_MODULE}")
  set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_COMMAND> -E vs_link_exe ${CMAKE_CXX_LINK_EXECUTABLE}")
  set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_COMMAND> -E vs_link_exe ${CMAKE_C_LINK_EXECUTABLE}")

  set(CMAKE_BUILD_TYPE_INIT Debug)
  set(CMAKE_CXX_FLAGS_INIT "/DWIN32 /D_WINDOWS /W3 /Zm1000 /EHsc /GR")
  set(CMAKE_CXX_FLAGS_DEBUG_INIT "/D_DEBUG /MDd /Zi /Ob0 /Od /RTC1")
  set(CMAKE_CXX_FLAGS_MINSIZEREL_INIT "/MD /O1 /Ob1 /D NDEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE_INIT "/MD /O2 /Ob2 /D NDEBUG")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "/MD /Zi /O2 /Ob1 /D NDEBUG")
  set(CMAKE_C_FLAGS_INIT "/DWIN32 /D_WINDOWS /W3 /Zm1000")
  set(CMAKE_C_FLAGS_DEBUG_INIT "/D_DEBUG /MDd /Zi  /Ob0 /Od /RTC1")
  set(CMAKE_C_FLAGS_MINSIZEREL_INIT "/MD /O1 /Ob1 /D NDEBUG")
  set(CMAKE_C_FLAGS_RELEASE_INIT "/MD /O2 /Ob2 /D NDEBUG")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO_INIT "/MD /Zi /O2 /Ob1 /D NDEBUG")
  set(CMAKE_C_STANDARD_LIBRARIES_INIT "kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib ")
  set(CMAKE_EXE_LINKER_FLAGS_INIT "${CMAKE_EXE_LINKER_FLAGS_INIT}")
else(MSVC_VERSION GREATER 1310)
  if(CMAKE_USING_VC_FREE_TOOLS)
    message(STATUS "Using FREE VC TOOLS, NO DEBUG available")
    set(CMAKE_BUILD_TYPE_INIT Release)
    set(CMAKE_CXX_FLAGS_INIT "/DWIN32 /D_WINDOWS /W3 /Zm1000 /GX /GR")
    set(CMAKE_CXX_FLAGS_DEBUG_INIT "/D_DEBUG /MTd /Zi  /Ob0 /Od /GZ")
    set(CMAKE_CXX_FLAGS_MINSIZEREL_INIT "/MT /O1 /Ob1 /D NDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE_INIT "/MT /O2 /Ob2 /D NDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "/MT /Zi /O2 /Ob1 /D NDEBUG")
    set(CMAKE_C_FLAGS_INIT "/DWIN32 /D_WINDOWS /W3 /Zm1000")
    set(CMAKE_C_FLAGS_DEBUG_INIT "/D_DEBUG /MTd /Zi  /Ob0 /Od /GZ")
    set(CMAKE_C_FLAGS_MINSIZEREL_INIT "/MT /O1 /Ob1 /D NDEBUG")
    set(CMAKE_C_FLAGS_RELEASE_INIT "/MT /O2 /Ob2 /D NDEBUG")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO_INIT "/MT /Zi /O2 /Ob1 /D NDEBUG")
  else(CMAKE_USING_VC_FREE_TOOLS)
    set(CMAKE_BUILD_TYPE_INIT Debug)
    set(CMAKE_CXX_FLAGS_INIT "/DWIN32 /D_WINDOWS /W3 /Zm1000 /GX /GR")
    set(CMAKE_CXX_FLAGS_DEBUG_INIT "/D_DEBUG /MDd /Zi  /Ob0 /Od /GZ")
    set(CMAKE_CXX_FLAGS_MINSIZEREL_INIT "/MD /O1 /Ob1 /D NDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE_INIT "/MD /O2 /Ob2 /D NDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "/MD /Zi /O2 /Ob1 /D NDEBUG")
    set(CMAKE_C_FLAGS_INIT "/DWIN32 /D_WINDOWS /W3 /Zm1000")
    set(CMAKE_C_FLAGS_DEBUG_INIT "/D_DEBUG /MDd /Zi /Ob0 /Od /GZ")
    set(CMAKE_C_FLAGS_MINSIZEREL_INIT "/MD /O1 /Ob1 /D NDEBUG")
    set(CMAKE_C_FLAGS_RELEASE_INIT "/MD /O2 /Ob2 /D NDEBUG")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO_INIT "/MD /Zi /O2 /Ob1 /D NDEBUG")
  endif(CMAKE_USING_VC_FREE_TOOLS)
  set(CMAKE_C_STANDARD_LIBRARIES_INIT "kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib")
endif(MSVC_VERSION GREATER 1310)

set(CMAKE_CXX_STANDARD_LIBRARIES_INIT "${CMAKE_C_STANDARD_LIBRARIES_INIT}")

# executable linker flags
set(CMAKE_LINK_DEF_FILE_FLAG "/DEF:")
# set the stack size and the machine type
set(_MACHINE_ARCH_FLAG ${MSVC_C_ARCHITECTURE_ID})
if(NOT _MACHINE_ARCH_FLAG)
  set(_MACHINE_ARCH_FLAG ${MSVC_CXX_ARCHITECTURE_ID})
endif(NOT _MACHINE_ARCH_FLAG)
set(CMAKE_EXE_LINKER_FLAGS_INIT
    "${CMAKE_EXE_LINKER_FLAGS_INIT} /STACK:10000000 /machine:${_MACHINE_ARCH_FLAG}")

# add /debug and /INCREMENTAL:YES to DEBUG and RELWITHDEBINFO also add pdbtype
# on versions that support it
set( MSVC_INCREMENTAL_YES_FLAG "")
if(NOT MSVC_INCREMENTAL_DEFAULT)
  set( MSVC_INCREMENTAL_YES_FLAG "/INCREMENTAL:YES")
else()
  set(  MSVC_INCREMENTAL_YES_FLAG "/INCREMENTAL" )
endif()

if(CMAKE_COMPILER_SUPPORTS_PDBTYPE)
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG_INIT "/debug /pdbtype:sept ${MSVC_INCREMENTAL_YES_FLAG}")
  set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO_INIT "/debug /pdbtype:sept ${MSVC_INCREMENTAL_YES_FLAG}")
else(CMAKE_COMPILER_SUPPORTS_PDBTYPE)
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG_INIT "/debug ${MSVC_INCREMENTAL_YES_FLAG}")
  set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO_INIT "/debug ${MSVC_INCREMENTAL_YES_FLAG}")
endif(CMAKE_COMPILER_SUPPORTS_PDBTYPE)
# for release and minsize release default to no incremental linking
set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL_INIT "/INCREMENTAL:NO")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE_INIT "/INCREMENTAL:NO")

# copy the EXE_LINKER flags to SHARED and MODULE linker flags
# shared linker flags
set(CMAKE_SHARED_LINKER_FLAGS_INIT ${CMAKE_EXE_LINKER_FLAGS_INIT})
set(CMAKE_SHARED_LINKER_FLAGS_DEBUG_INIT ${CMAKE_EXE_LINKER_FLAGS_DEBUG_INIT})
set(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO_INIT ${CMAKE_EXE_LINKER_FLAGS_DEBUG_INIT})
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE_INIT ${CMAKE_EXE_LINKER_FLAGS_RELEASE_INIT})
set(CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL_INIT ${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL_INIT})
# module linker flags
set(CMAKE_MODULE_LINKER_FLAGS_INIT ${CMAKE_SHARED_LINKER_FLAGS_INIT})
set(CMAKE_MODULE_LINKER_FLAGS_DEBUG_INIT ${CMAKE_SHARED_LINKER_FLAGS_DEBUG_INIT})
set(CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO_INIT ${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO_INIT})
set(CMAKE_MODULE_LINKER_FLAGS_RELEASE_INIT ${CMAKE_EXE_LINKER_FLAGS_RELEASE_INIT})
set(CMAKE_MODULE_LINKER_FLAGS_MINSIZEREL_INIT ${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL_INIT})

# save computed information for this platform
if(NOT EXISTS "${CMAKE_PLATFORM_ROOT_BIN}/CMakeCPlatform.cmake")
  configure_file(${CMAKE_ROOT}/Modules/Platform/Windows-cl.cmake.in
    ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeCPlatform.cmake IMMEDIATE)
endif(NOT EXISTS "${CMAKE_PLATFORM_ROOT_BIN}/CMakeCPlatform.cmake")

if(NOT EXISTS "${CMAKE_PLATFORM_ROOT_BIN}/CMakeCXXPlatform.cmake")
  configure_file(${CMAKE_ROOT}/Modules/Platform/Windows-cl.cmake.in
               ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeCXXPlatform.cmake IMMEDIATE)
endif(NOT EXISTS "${CMAKE_PLATFORM_ROOT_BIN}/CMakeCXXPlatform.cmake")
