# microsoft specific config file 

FIND_PATH(BCB_BIN_PATH bcc32.exe
  "C:/Program Files/Borland/CBuilder5/Bin"
  "C:/Borland/Bcc55/Bin"
  "/Borland/Bcc55/Bin"
  [HKEY_LOCAL_MACHINE/SOFTWARE/Borland/C++Builder/5.0/RootDir]/Bin
)
SET (BORLAND 1)
SET (WORDS_BIGENDIAN )
SET (HAVE_LIMITS_H   1)
SET (HAVE_UNISTD_H   1)
SET (CMAKE_CXX_COMPILER  "${BCB_BIN_PATH}/bcc32" CACHE FILEPATH
     "Name of C++ compiler used.")
SET (CMAKE_C_COMPILER ${BCB_BIN_PATH}/bcc32  CACHE FILEPATH
     "Name of C compiler used.")
SET (CMAKE_CFLAGS  "-w- -whid -waus -wpar" CACHE STRING
     "Flags for C compiler.")
SET (CMAKE_BUILD_TYPE Debug CACHE STRING 
"Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel")
SET (CMAKE_CXX_FLAGS_RELEASE "-O2" CACHE STRING
        "Flags used by the compiler during release builds.)")
SET (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Od" CACHE STRING
        "Flags used by the compiler during Release with Debug Info builds")
SET (CMAKE_CXX_FLAGS_MINSIZEREL "-O1" CACHE STRING
        "Flags used by the compiler during release minsize builds")
SET (CMAKE_CXX_FLAGS_DEBUG "-Od" CACHE STRING
        "Flags used by the compiler during debug builds")
SET (CMAKE_CXX_FLAGS "-w- -whid -waus -wpar" CACHE STRING
        "Flags used by the compiler during all build types, /GX /GR are for exceptions and rtti in VC++, /Zm1000 increases the compiler's memory allocation to support ANSI C++/stdlib")
SET (CMAKE_USE_WIN32_THREADS 1 CACHE BOOL "Use the win32 thread library")
SET (CMAKE_STANDARD_WINDOWS_LIBRARIES "import32.lib"
         CACHE STRING "Libraries linked by defalut with all applications")
SET (CMAKE_SHLIB_SUFFIX       ".dll" CACHE STRING "Shared library suffix")
SET (CMAKE_MODULE_SUFFIX      ".dll" CACHE STRING "Module library suffix")

FIND_PROGRAM(CMAKE_MAKE_PROGRAM make ${BCB_BIN_PATH} )



