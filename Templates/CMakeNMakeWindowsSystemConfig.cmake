# microsoft specific config file 
SET (WORDS_BIGENDIAN )
SET (HAVE_LIMITS_H   1)
SET (HAVE_UNISTD_H   1)
SET (CMAKE_CXX_COMPILER  cl CACHE FILEPATH
     "Name of C++ compiler used.")
SET (CMAKE_C_COMPILER  cl CACHE FILEPATH
     "Name of C compiler used.")
SET (CMAKE_CFLAGS  "/W3 /Zm1000" CACHE STRING
     "Flags for C compiler.")
SET (CMAKE_BUILD_TYPE Debug CACHE STRING 
"Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel")
SET (CMAKE_CXX_FLAGS_RELEASE "/MD /O2" CACHE STRING
        "Flags used by the compiler during release builds (/MD /Ob1 /Oi /Ot /Oy /Gs will produce slightly less optimized but smaller files)")
SET (CMAKE_CXX_FLAGS_RELWITHDEBINFO "/MD /Zi /O2" CACHE STRING
        "Flags used by the compiler during Release with Debug Info builds")
SET (CMAKE_CXX_FLAGS_MINSIZEREL "/MD /O1" CACHE STRING
        "Flags used by the compiler during release minsize builds")
SET (CMAKE_CXX_FLAGS_DEBUG "/MDd /Zi /Od /GZ" CACHE STRING
        "Flags used by the compiler during debug builds")
SET (CMAKE_CXX_FLAGS "/W3 /Zm1000 /GX /GR" CACHE STRING
        "Flags used by the compiler during all build types, /GX /GR are for exceptions and rtti in VC++, /Zm1000 increases the compiler's memory allocation to support ANSI C++/stdlib")
SET (CMAKE_USE_WIN32_THREADS 1 CACHE BOOL "Use the win32 thread library")
SET (CMAKE_STANDARD_WINDOWS_LIBRARIES "kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib" CACHE STRING "Libraries linked by defalut with all applications")
SET (CMAKE_SHLIB_SUFFIX       ".dll" CACHE STRING "Shared library suffix")
SET (CMAKE_MODULE_SUFFIX      ".dll" CACHE STRING "Module library suffix")
SET (CMAKE_MAKE_PROGRAM      "nmake" CACHE STRING "Program used to build from makefiles.")




