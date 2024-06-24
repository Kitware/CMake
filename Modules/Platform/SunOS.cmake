if(CMAKE_SYSTEM MATCHES "SunOS-4")
  set(CMAKE_C_COMPILE_OPTIONS_PIC "-PIC")
  set(CMAKE_C_COMPILE_OPTIONS_PIE "-PIE")
  set(CMAKE_SHARED_LIBRARY_C_FLAGS "-PIC")
  set(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "-shared -Wl,-r")
  set(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG "-Wl,-R")
  set(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG_SEP ":")
endif()


# Features for LINK_LIBRARY generator expression
## WHOLE_ARCHIVE: Force loading all members of an archive
if (CMAKE_SYSTEM_VERSION VERSION_GREATER "5.10")
  set(CMAKE_LINK_LIBRARY_USING_WHOLE_ARCHIVE "LINKER:--whole-archive"
                                             "<LINK_ITEM>"
                                             "LINKER:--no-whole-archive")
else()
  set(CMAKE_LINK_LIBRARY_USING_WHOLE_ARCHIVE "LINKER:-z,allextract"
                                             "<LINK_ITEM>"
                                             "LINKER:-z,defaultextract")
endif()
set(CMAKE_LINK_LIBRARY_USING_WHOLE_ARCHIVE_SUPPORTED TRUE)
set(CMAKE_LINK_LIBRARY_WHOLE_ARCHIVE_ATTRIBUTES LIBRARY_TYPE=STATIC DEDUPLICATION=YES OVERRIDE=DEFAULT)


# Features for LINK_GROUP generator expression
if (CMAKE_SYSTEM_VERSION VERSION_GREATER "5.9")
  ## RESCAN: request the linker to rescan static libraries until there is
  ## no pending undefined symbols
  set(CMAKE_LINK_GROUP_USING_RESCAN "LINKER:-z,rescan-start" "LINKER:-z,rescan-end")
  set(CMAKE_LINK_GROUP_USING_RESCAN_SUPPORTED TRUE)
endif()


include(Platform/UnixPaths)

list(APPEND CMAKE_SYSTEM_PREFIX_PATH
  /opt/csw
  /opt/openwin
  )

# The Sun linker needs to find transitive shared library dependencies
# in the -L path.
set(CMAKE_LINK_DEPENDENT_LIBRARY_DIRS 1)

# Shared libraries with no builtin soname may not be linked safely by
# specifying the file path.
set(CMAKE_PLATFORM_USES_PATH_WHEN_NO_SONAME 1)
