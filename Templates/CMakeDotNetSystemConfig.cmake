# Microsoft specific config file 

# try to find the devenv executable because 
# visual studio 7 does not automatically put it in your PATH
FIND_PROGRAM(MICROSOFT_DEVENV
        NAMES devenv
        PATHS
        [HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\7.0\\Setup\\VS;EnvironmentDirectory]
        "c:/Program Files/Microsoft Visual Studio .NET/Common7/IDE"
        "c:/Program Files/Microsoft Visual Studio.NET/Common7/IDE"
)

SET (CMAKE_BUILD_TOOL devenv CACHE INTERNAL 
     "What is the target build tool cmake is generating for.")

SET (CMAKE_SYSTEM "Win32" CACHE INTERNAL 
     "What system is this.  Result of uname.")

SET (CMAKE_CXX_COMPILER  cl CACHE STRING 
     "Name of C++ compiler used.")

SET (CMAKE_CXX_FLAGS "/Zm1000 " CACHE STRING
     "Flags used by the compiler during all build types, /Zm1000 increases the compiler's memory allocation to support ANSI C++/stdlib, /W3 sets the warning level to 3")

SET (CMAKE_CXX_STACK_SIZE "10000000" CACHE STRING
     "Size of stack for programs.")

SET (CMAKE_CXX_WARNING_LEVEL "3" CACHE STRING
     "Size of stack for programs.")

SET (CMAKE_CXX_USE_RTTI 1 CACHE BOOL 
     "Compile CXX code with run time type information.")

SET (CMAKE_USE_WIN32_THREADS 1 CACHE BOOL 
     "Use the win32 thread library")

SET (CMAKE_MAKE_PROGRAM "${MICROSOFT_DEVENV}" CACHE STRING 
     "Program used to build from project files.")

SET (BUILDNAME "Win32-DotNET-devenv" CACHE STRING 
     "Name used by dart to specify the build name.")

SET (CMAKE_CONFIGURATION_TYPES "Debug Release MinSizeRel RelWithDebInfo" CACHE STRING 
     "Space separated list of supported configuration types, only supports Debug, Release, MinSizeRel, and RelWithDebInfo, anything else will be ignored.")

# We will hardcode them for now. Make sure to fix that in the future
SET (CMAKE_SIZEOF_INT       4   CACHE INTERNAL "Size of int data type")
SET (CMAKE_SIZEOF_LONG      4   CACHE INTERNAL "Size of long data type")
SET (CMAKE_SIZEOF_VOID_P    4   CACHE INTERNAL "Size of void* data type")
SET (CMAKE_SIZEOF_CHAR      1   CACHE INTERNAL "Size of char data type")
SET (CMAKE_SIZEOF_SHORT     2   CACHE INTERNAL "Size of short data type")
SET (CMAKE_SIZEOF_FLOAT     4   CACHE INTERNAL "Size of float data type")
SET (CMAKE_SIZEOF_DOUBLE    8   CACHE INTERNAL "Size of double data type")

# Suffixes

SET (CMAKE_EXECUTABLE_SUFFIX ".exe" CACHE INTERNAL
     "Executable suffix.")

SET (CMAKE_MODULE_SUFFIX ".dll" CACHE INTERNAL 
     "Module library suffix.")

SET (CMAKE_OBJECT_FILE_SUFFIX ".obj" CACHE INTERNAL 
     "Object file suffix.")

SET (CMAKE_SHLIB_SUFFIX ".dll" CACHE INTERNAL 
     "Shared library suffix.")

SET (CMAKE_STATICLIB_SUFFIX ".lib" CACHE INTERNAL 
     "Static library suffix.")

# The following variables are advanced 

MARK_AS_ADVANCED(
BUILDNAME
CMAKE_CXX_USE_RTTI
CMAKE_CXX_COMPILER
CMAKE_CXX_STACK_SIZE
CMAKE_CXX_WARNING_LEVEL
CMAKE_USE_WIN32_THREADS
CMAKE_MAKE_PROGRAM
CMAKE_EXTRA_LINK_FLAGS
MICROSOFT_DEVENV
CMAKE_EXECUTABLE_SUFFIX
CMAKE_MODULE_SUFFIX
CMAKE_OBJECT_FILE_SUFFIX
CMAKE_SHLIB_SUFFIX
CMAKE_STATICLIB_SUFFIX
)
