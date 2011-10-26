set(CMAKE_LIBRARY_PATH_FLAG "libpath ")
set(CMAKE_LINK_LIBRARY_FLAG "library ")
set(CMAKE_LINK_LIBRARY_FILE_FLAG "library")

if(CMAKE_VERBOSE_MAKEFILE)
  set(CMAKE_WCL_QUIET)
  set(CMAKE_WLINK_QUIET)
  set(CMAKE_LIB_QUIET)
else(CMAKE_VERBOSE_MAKEFILE)
  set(CMAKE_WCL_QUIET "-zq")
  set(CMAKE_WLINK_QUIET "option quiet")
  set(CMAKE_LIB_QUIET "-q")
endif(CMAKE_VERBOSE_MAKEFILE)

set(CMAKE_CREATE_WIN32_EXE "system nt_win" )
set(CMAKE_CREATE_CONSOLE_EXE "system nt" )

set(CMAKE_EXE_LINKER_FLAGS_DEBUG_INIT "debug all" )
set(CMAKE_SHARED_LINKER_FLAGS_DEBUG_INIT "debug all" )
set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO_INIT "debug all" )
set(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO_INIT "debug all" )

set (CMAKE_SHARED_LIBRARY_C_FLAGS "-bd" )

set(CMAKE_RC_COMPILER "rc" )

set(CMAKE_BUILD_TYPE_INIT Debug)
set(CMAKE_CXX_FLAGS_INIT "-w=3 -xs")
set(CMAKE_CXX_FLAGS_DEBUG_INIT "-br -bm -d2")
set(CMAKE_CXX_FLAGS_MINSIZEREL_INIT "-br -bm -os -dNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-br -bm -ot -dNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "-br -bm  -d2 -ot -dNDEBUG")
set(CMAKE_C_FLAGS_INIT "-w=3 ")
set(CMAKE_C_FLAGS_DEBUG_INIT "-br -bm -d2 -od")
set(CMAKE_C_FLAGS_MINSIZEREL_INIT "-br -bm -os -dNDEBUG")
set(CMAKE_C_FLAGS_RELEASE_INIT "-br -bm -ot -dNDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO_INIT "-br -bm -d2 -ot -dNDEBUG")
set(CMAKE_C_STANDARD_LIBRARIES_INIT "library clbrdll.lib library plbrdll.lib  library kernel32.lib library user32.lib library gdi32.lib library winspool.lib library comdlg32.lib library advapi32.lib library shell32.lib library ole32.lib library oleaut32.lib library uuid.lib library odbc32.lib library odbccp32.lib")
set(CMAKE_CXX_STANDARD_LIBRARIES_INIT "${CMAKE_C_STANDARD_LIBRARIES_INIT}")

set(CMAKE_C_CREATE_IMPORT_LIBRARY
  "wlib -c -q -n -b <TARGET_IMPLIB> +'<TARGET_UNQUOTED>'")
set(CMAKE_CXX_CREATE_IMPORT_LIBRARY ${CMAKE_C_CREATE_IMPORT_LIBRARY})

set(CMAKE_C_LINK_EXECUTABLE
    "wlink ${CMAKE_START_TEMP_FILE} ${CMAKE_WLINK_QUIET} name '<TARGET_UNQUOTED>' <LINK_FLAGS> option caseexact file {<OBJECTS>} <LINK_LIBRARIES> ${CMAKE_END_TEMP_FILE}")


set(CMAKE_CXX_LINK_EXECUTABLE ${CMAKE_C_LINK_EXECUTABLE})

# compile a C++ file into an object file
set(CMAKE_CXX_COMPILE_OBJECT
    "<CMAKE_CXX_COMPILER> ${CMAKE_START_TEMP_FILE} ${CMAKE_WCL_QUIET} <FLAGS> -dWIN32 -d+ <DEFINES> -fo<OBJECT> -c -cc++ <SOURCE>${CMAKE_END_TEMP_FILE}")

# compile a C file into an object file
set(CMAKE_C_COMPILE_OBJECT
    "<CMAKE_C_COMPILER> ${CMAKE_START_TEMP_FILE} ${CMAKE_WCL_QUIET} <FLAGS> -dWIN32 -d+ <DEFINES> -fo<OBJECT> -c -cc <SOURCE>${CMAKE_END_TEMP_FILE}")

# preprocess a C source file
set(CMAKE_C_CREATE_PREPROCESSED_SOURCE
    "<CMAKE_C_COMPILER> ${CMAKE_START_TEMP_FILE} ${CMAKE_WCL_QUIET} <FLAGS> -dWIN32 -d+ <DEFINES> -fo<PREPROCESSED_SOURCE> -pl -cc <SOURCE>${CMAKE_END_TEMP_FILE}")

# preprocess a C++ source file
set(CMAKE_CXX_CREATE_PREPROCESSED_SOURCE
    "<CMAKE_CXX_COMPILER> ${CMAKE_START_TEMP_FILE} ${CMAKE_WCL_QUIET} <FLAGS> -dWIN32 -d+ <DEFINES> -fo<PREPROCESSED_SOURCE> -pl -cc++ <SOURCE>${CMAKE_END_TEMP_FILE}")

set(CMAKE_CXX_CREATE_SHARED_MODULE
 "wlink ${CMAKE_START_TEMP_FILE} system nt_dll  ${CMAKE_WLINK_QUIET} name '<TARGET_UNQUOTED>' <LINK_FLAGS> option caseexact  file {<OBJECTS>} <LINK_LIBRARIES> ${CMAKE_END_TEMP_FILE}")
set(CMAKE_CXX_CREATE_SHARED_LIBRARY
  ${CMAKE_CXX_CREATE_SHARED_MODULE}
  ${CMAKE_CXX_CREATE_IMPORT_LIBRARY})

# create a C shared library
set(CMAKE_C_CREATE_SHARED_LIBRARY ${CMAKE_CXX_CREATE_SHARED_LIBRARY})

# create a C shared module
set(CMAKE_C_CREATE_SHARED_MODULE ${CMAKE_CXX_CREATE_SHARED_MODULE})

# create a C++ static library
set(CMAKE_CXX_CREATE_STATIC_LIBRARY  "wlib ${CMAKE_LIB_QUIET} -c -n -b '<TARGET_UNQUOTED>' <LINK_FLAGS> <OBJECTS> ")

# create a C static library
set(CMAKE_C_CREATE_STATIC_LIBRARY ${CMAKE_CXX_CREATE_STATIC_LIBRARY})

if(NOT CMAKE_WATCOM_COMPILER_TESTS_RUN)
  set(CMAKE_WATCOM_COMPILER_TESTS_RUN 1)
  set(testWatcomVersionFile
    "${CMAKE_ROOT}/Modules/CMakeTestWatcomVersion.c")
  string(REGEX REPLACE "/" "\\\\" testWatcomVersionFile "${testWatcomVersionFile}")
  message(STATUS "Check for Watcom compiler version")
  set(CMAKE_TEST_COMPILER ${CMAKE_C_COMPILER})
  if(NOT CMAKE_C_COMPILER)
    set(CMAKE_TEST_COMPILER ${CMAKE_CXX_COMPILER})
  endif()
  execute_process(COMMAND ${CMAKE_TEST_COMPILER}
    -q -pc \"${testWatcomVersionFile}\"
    OUTPUT_VARIABLE CMAKE_COMPILER_OUTPUT
    RESULT_VARIABLE CMAKE_COMPILER_RETURN
    )
  string(REGEX REPLACE "\n" " " compilerVersion "${CMAKE_COMPILER_OUTPUT}")
  string(REGEX REPLACE ".*VERSION=(.*)" "\\1"
    compilerVersion "${compilerVersion}")
  if("${CMAKE_COMPILER_RETURN}" STREQUAL "0")
    set(WATCOM16)
    set(WATCOM17)
    set(WATCOM18)
    set(WATCOM19)
    if("${compilerVersion}" LESS 1270)
      set(WATCOM16 1)
    endif()
    if("${compilerVersion}" EQUAL 1270)
      set(WATCOM17 1)
    endif()
    if("${compilerVersion}" EQUAL 1280)
      set(WATCOM18 1)
    endif()
    if("${compilerVersion}" EQUAL 1290)
      set(WATCOM19 1)
    endif()
  endif()
endif()
