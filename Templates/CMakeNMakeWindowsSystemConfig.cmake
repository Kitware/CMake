# Microsoft specific config file 
# NOTE: all entries in here MUST be CACHE values, regular SET will NOT WORK!


# Suffixes

SET (CMAKE_SYSTEM "Win32" CACHE INTERNAL 
     "What system is this.  Result of uname.")

SET (CMAKE_BUILD_TOOL nmake CACHE INTERNAL 
     "What is the target build tool cmake is generating for.")

SET (CMAKE_EXECUTABLE_SUFFIX ".exe" CACHE STRING 
     "Executable suffix.")

SET (CMAKE_MODULE_SUFFIX ".dll" CACHE STRING 
     "Module library suffix.")

SET (CMAKE_OBJECT_FILE_SUFFIX ".obj" CACHE STRING 
     "Object file suffix.")

SET (CMAKE_SHLIB_SUFFIX ".dll" CACHE STRING 
     "Shared library suffix.")

SET (CMAKE_STATICLIB_SUFFIX ".lib" CACHE STRING 
     "Static library suffix.")

# ANSI

SET (CMAKE_ANSI_CFLAGS "" CACHE INTERNAL 
     "What flags are required by the c++ compiler to make it ansi.")

# We will hardcode them for now. Make sure to fix that in the future
SET (CMAKE_SIZEOF_INT       4   CACHE INTERNAL "Size of int data type")
SET (CMAKE_SIZEOF_LONG      4   CACHE INTERNAL "Size of long data type")
SET (CMAKE_SIZEOF_VOID_P    4   CACHE INTERNAL "Size of void* data type")
SET (CMAKE_SIZEOF_CHAR      1   CACHE INTERNAL "Size of char data type")
SET (CMAKE_SIZEOF_SHORT     2   CACHE INTERNAL "Size of short data type")
SET (CMAKE_SIZEOF_FLOAT     4   CACHE INTERNAL "Size of float data type")
SET (CMAKE_SIZEOF_DOUBLE    8   CACHE INTERNAL "Size of double data type")

# Build type

SET (CMAKE_BUILD_TYPE Debug CACHE STRING 
     "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel.")

# C++

SET (CMAKE_CXX_COMPILER cl CACHE FILEPATH
     "Name of C++ compiler used.")

SET (CMAKE_CXX_FLAGS "/W3 /Zm1000 /GX /GR" CACHE STRING
     "Flags used by the compiler during all build types, /GX /GR are for exceptions and rtti in VC++, /Zm1000 increases the compiler's memory allocation to support ANSI C++/stdlib.")

SET (CMAKE_CXX_FLAGS_DEBUG "/MDd /Zi /Od /GZ" CACHE STRING
     "Flags used by the compiler during debug builds.")

SET (CMAKE_CXX_FLAGS_MINSIZEREL "/MD /O1" CACHE STRING
     "Flags used by the compiler during release minsize builds.")

SET (CMAKE_CXX_FLAGS_RELEASE "/MD /O2" CACHE STRING
     "Flags used by the compiler during release builds (/MD /Ob1 /Oi /Ot /Oy /Gs will produce slightly less optimized but smaller files).")

SET (CMAKE_CXX_FLAGS_RELWITHDEBINFO "/MD /Zi /O2" CACHE STRING
     "Flags used by the compiler during Release with Debug Info builds.")

# C

SET (CMAKE_C_COMPILER cl CACHE FILEPATH
     "Name of C compiler used.")

SET (CMAKE_C_FLAGS "/W3 /Zm1000" CACHE STRING
     "Flags for C compiler.")

SET (CMAKE_C_LIBPATH_FLAG "-LIBPATH:" CACHE STRING
     "Flags used to specify a link path. No space will be appended (use single quotes around value to insert trailing space).")

SET (CMAKE_C_LINK_EXECUTABLE_FLAG "/link" CACHE STRING
     "Flags used to create an executable.")

SET (CMAKE_C_OUTPUT_EXECUTABLE_FILE_FLAG "/Fe" CACHE STRING
     "Flags used to specify executable filename. No space will be appended (use single quotes around value to insert trailing space).")

SET (CMAKE_C_OUTPUT_OBJECT_FILE_FLAG "/Fo" CACHE STRING
     "Flags used to specify output filename. No space will be appended (use single quotes around value to insert trailing space).")

# Library manager

SET (CMAKE_LIBRARY_MANAGER lib CACHE FILEPATH
     "Name of the library manager used (static lib).")

SET (CMAKE_LIBRARY_MANAGER_FLAGS "/nologo" CACHE STRING
     "Flags used by the library manager.")

SET (CMAKE_LIBRARY_MANAGER_OUTPUT_FILE_FLAG "/out:" CACHE STRING
     "Flags used to specify output filename by the library manager. No space will be appended (use single quotes around value to insert trailing space).")

# Linker (DLL/exe)

SET (CMAKE_LINKER link CACHE FILEPATH
     "Name of linker used.")

SET (CMAKE_LINKER_FLAGS "/nologo /STACK:10000000 /machine:I386" CACHE STRING
     "Flags used by the linker.")

SET (CMAKE_LINKER_FLAGS_DEBUG "/debug /pdbtype:sept" CACHE STRING
     "Flags used by the linker during debug builds.")

SET (CMAKE_LINKER_FLAGS_MINSIZEREL "" CACHE STRING
     "Flags used by the linker during release minsize builds.")

SET (CMAKE_LINKER_FLAGS_RELEASE "" CACHE STRING
     "Flags used by the linker during release builds.")

SET (CMAKE_LINKER_FLAGS_RELWITHDEBINFO "/debug /pdbtype:sept" CACHE STRING
     "Flags used by the linker during Release with Debug Info builds.")

SET (CMAKE_LINKER_HIDE_PARAMETERS 1 CACHE BOOL 
     "Hide linker parameters when it is run.")

SET (CMAKE_LINKER_OUTPUT_FILE_FLAG "/out:" CACHE STRING
     "Flags used to specify output filename by the linker. No space will be appended (use single quotes around value to insert trailing space).")

SET (CMAKE_LINKER_SHARED_LIBRARY_FLAG "/dll" CACHE STRING
     "Flags used to create a shared library.")

# Standard lib

SET (CMAKE_STANDARD_WINDOWS_LIBRARIES "kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib" CACHE STRING 
     "Libraries linked by defalut with all applications.")

# Make

SET (CMAKE_MAKE_PROGRAM "nmake" CACHE STRING 
     "Program used to build from makefiles.")

# Threads

SET (CMAKE_USE_WIN32_THREADS 1 CACHE BOOL 
     "Use the win32 thread library.")

# this should not be advanced, but was so we have to 
# clear it in all the cache files
MARK_AS_ADVANCED( CLEAR CMAKE_BUILD_TYPE)

# The following variables are advanced 


MARK_AS_ADVANCED(
WORDS_BIGENDIAN
HAVE_LIMITS_H
HAVE_UNISTD_H
CMAKE_EXECUTABLE_SUFFIX
CMAKE_MODULE_SUFFIX
CMAKE_OBJECT_FILE_SUFFIX
CMAKE_SHLIB_SUFFIX
CMAKE_STATICLIB_SUFFIX
CMAKE_ANSI_CFLAGS
CMAKE_CXX_COMPILER
CMAKE_CXX_FLAGS
CMAKE_CXX_FLAGS_DEBUG
CMAKE_CXX_FLAGS_MINSIZEREL
CMAKE_CXX_FLAGS_RELEASE
CMAKE_CXX_FLAGS_RELWITHDEBINFO
CMAKE_C_COMPILER
CMAKE_C_FLAGS
CMAKE_C_LIBPATH_FLAG
CMAKE_C_LINK_EXECUTABLE_FLAG
CMAKE_C_OUTPUT_EXECUTABLE_FILE_FLAG
CMAKE_C_OUTPUT_OBJECT_FILE_FLAG
CMAKE_LIBRARY_MANAGER
CMAKE_LIBRARY_MANAGER_FLAGS
CMAKE_LIBRARY_MANAGER_OUTPUT_FILE_FLAG
CMAKE_LINKER
CMAKE_LINKER_FLAGS
CMAKE_LINKER_FLAGS_DEBUG
CMAKE_LINKER_FLAGS_MINSIZEREL
CMAKE_LINKER_FLAGS_RELEASE
CMAKE_LINKER_FLAGS_RELWITHDEBINFO
CMAKE_LINKER_HIDE_PARAMETERS
CMAKE_LINKER_OUTPUT_FILE_FLAG
CMAKE_LINKER_SHARED_LIBRARY_FLAG
CMAKE_MAKE_PROGRAM
CMAKE_STANDARD_WINDOWS_LIBRARIES
CMAKE_USE_WIN32_THREADS
)
