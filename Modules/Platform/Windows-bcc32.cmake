# Borland shared library issues:
# When building dll's with borland, the run time dll c/c++ library from
# borland must be used.   This is specified with the -tWR compiler option.
# This flag must be present during compilation of c and c++ files and
# for the linking of exe and dll files.   But wait, there is more,
# the -tWR flag must come after the  -tWD and -tWM flags, but before the -tWC flag.
# Order counts, so be careful!
# if building static, you don't want the -tWR flag as it will make your program
# depend on the borland run time dll.
# So, if a project has CMAKE_BUILD_SHARED on, then the -tWR flag is added all over, and
# it is left out if not.

SET(CMAKE_LIBRARY_PATH_FLAG "-L")
SET(CMAKE_LINK_LIBRARY_FLAG "")
SET(CMAKE_SHARED_BUILD_CXX_FLAGS "-tWR")
SET(CMAKE_SHARED_BUILD_C_FLAGS "-tWR")
SET(BORLAND 1)

SET(CMAKE_FIND_LIBRARY_SUFFIXES "-bcc.lib" ".lib")

# uncomment these out to debug makefiles
#SET(CMAKE_START_TEMP_FILE "")
#SET(CMAKE_END_TEMP_FILE "")
#SET(CMAKE_VERBOSE_MAKEFILE 1)

# Borland cannot handle + in the file name, so mangle object file name
SET (CMAKE_MANGLE_OBJECT_FILE_NAMES "ON")

# Create an import library for another target.
SET(CMAKE_CXX_CREATE_IMPORT_LIBRARY
  "implib -c -w <TARGET_IMPLIB> <TARGET>"
  )

# Create a C++ module library.
SET(CMAKE_CXX_CREATE_SHARED_MODULE
 "<CMAKE_CXX_COMPILER>  ${CMAKE_START_TEMP_FILE}-e<TARGET>  -tWD  <LINK_FLAGS> -tWR <LINK_LIBRARIES> <OBJECTS>${CMAKE_END_TEMP_FILE}"
)

# Create a C++ shared library.
# First create a module and then its import library.
SET(CMAKE_CXX_CREATE_SHARED_LIBRARY
  ${CMAKE_CXX_CREATE_SHARED_MODULE}
  ${CMAKE_CXX_CREATE_IMPORT_LIBRARY})

# Create an import library for another target.
SET(CMAKE_C_CREATE_IMPORT_LIBRARY ${CMAKE_CXX_CREATE_IMPORT_LIBRARY})

# Create a C module library.
SET(CMAKE_C_CREATE_SHARED_MODULE
 "<CMAKE_C_COMPILER> ${CMAKE_START_TEMP_FILE}-e<TARGET> -tWD  <LINK_FLAGS> -tWR <LINK_LIBRARIES> <OBJECTS>${CMAKE_END_TEMP_FILE}"
)

# Create a C shared library.
# First create a module and then its import library.
SET(CMAKE_C_CREATE_SHARED_LIBRARY
  ${CMAKE_C_CREATE_SHARED_MODULE}
  ${CMAKE_C_CREATE_IMPORT_LIBRARY})

# create a C++ static library
SET(CMAKE_CXX_CREATE_STATIC_LIBRARY  "tlib ${CMAKE_START_TEMP_FILE}/p512 <LINK_FLAGS> /a <TARGET> <OBJECTS>${CMAKE_END_TEMP_FILE}")

# create a C static library
SET(CMAKE_C_CREATE_STATIC_LIBRARY ${CMAKE_CXX_CREATE_STATIC_LIBRARY})

# compile a C++ file into an object file
# place <DEFINES> outside the response file because Borland refuses
# to parse quotes from the response file.
SET(CMAKE_CXX_COMPILE_OBJECT
    "<CMAKE_CXX_COMPILER> <DEFINES> ${CMAKE_START_TEMP_FILE}-DWIN32 -P <FLAGS> -o<OBJECT> -c <SOURCE>${CMAKE_END_TEMP_FILE}")

# compile a C file into an object file
# place <DEFINES> outside the response file because Borland refuses
# to parse quotes from the response file.
SET(CMAKE_C_COMPILE_OBJECT
    "<CMAKE_C_COMPILER> <DEFINES> ${CMAKE_START_TEMP_FILE}-DWIN32 -o<OBJECT> <FLAGS> -c <SOURCE>${CMAKE_END_TEMP_FILE}")


SET(CMAKE_C_LINK_EXECUTABLE
  "<CMAKE_C_COMPILER> ${CMAKE_START_TEMP_FILE}-e<TARGET> <LINK_FLAGS> <FLAGS> <LINK_LIBRARIES> <OBJECTS> ${CMAKE_END_TEMP_FILE}"
#  "implib -c -w <TARGET_IMPLIB> <TARGET>"
  )


SET(CMAKE_CXX_LINK_EXECUTABLE
  "<CMAKE_CXX_COMPILER> ${CMAKE_START_TEMP_FILE} <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> -e<TARGET> <FLAGS> <LINK_LIBRARIES> <OBJECTS> ${CMAKE_END_TEMP_FILE}"
#  "implib -c -w <TARGET_IMPLIB> <TARGET>"
  )

SET (CMAKE_BUILD_TYPE Debug CACHE STRING 
     "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel.")

# extra flags for a win32 exe
SET(CMAKE_CREATE_WIN32_EXE "-tW -tWM" )
# extra flags for a console app
SET(CMAKE_CREATE_CONSOLE_EXE "-tWC" )

SET (CMAKE_CXX_FLAGS_INIT "-tWM")
SET (CMAKE_CXX_FLAGS_DEBUG_INIT "-Od -v")
SET (CMAKE_CXX_FLAGS_MINSIZEREL_INIT "-O1 -DNDEBUG")
SET (CMAKE_CXX_FLAGS_RELEASE_INIT "-O2 -DNDEBUG")
SET (CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "-Od")
SET (CMAKE_C_FLAGS_INIT "-tWM")
SET (CMAKE_C_FLAGS_DEBUG_INIT "-Od -v")
SET (CMAKE_C_FLAGS_MINSIZEREL_INIT "-O1 -DNDEBUG")
SET (CMAKE_C_FLAGS_RELEASE_INIT "-O2 -DNDEBUG")
SET (CMAKE_C_FLAGS_RELWITHDEBINFO_INIT "-Od")

SET (CMAKE_EXE_LINKER_FLAGS_INIT "-tWM -lS:10000000 -lSc:10000000 ")
SET (CMAKE_EXE_LINKER_FLAGS_DEBUG_INIT "-v")
SET (CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO_INIT "-v")
SET (CMAKE_SHARED_LINKER_FLAGS_INIT ${CMAKE_EXE_LINKER_FLAGS_INIT})
SET (CMAKE_SHARED_LINKER_FLAGS_DEBUG_INIT ${CMAKE_EXE_LINKER_FLAGS_DEBUG_INIT})
SET (CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO_INIT ${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO_INIT})
SET (CMAKE_MODULE_LINKER_FLAGS_INIT ${CMAKE_SHARED_LINKER_FLAGS_INIT})
SET (CMAKE_MODULE_LINKER_FLAGS_DEBUG_INIT ${CMAKE_SHARED_LINKER_FLAGS_DEBUG_INIT})
SET (CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO_INIT ${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO_INIT})

SET (CMAKE_C_STANDARD_LIBRARIES_INIT "import32.lib")
SET (CMAKE_CXX_STANDARD_LIBRARIES_INIT "${CMAKE_C_STANDARD_LIBRARIES_INIT}")

# preprocess C and CXX files
# place <DEFINES> outside the response file because Borland refuses
# to parse quotes from the response file.
SET (CMAKE_C_CREATE_PREPROCESSED_SOURCE
    "cpp32 <DEFINES> ${CMAKE_START_TEMP_FILE}-DWIN32 <FLAGS> -o<PREPROCESSED_SOURCE> -c <SOURCE>${CMAKE_END_TEMP_FILE}")
SET (CMAKE_CXX_CREATE_PREPROCESSED_SOURCE
    "cpp32 <DEFINES> ${CMAKE_START_TEMP_FILE}-DWIN32 <FLAGS> -o<PREPROCESSED_SOURCE> -P -c <SOURCE>${CMAKE_END_TEMP_FILE}")
# Borland >= 5.6 allows -P option for cpp32, <= 5.5 does not
