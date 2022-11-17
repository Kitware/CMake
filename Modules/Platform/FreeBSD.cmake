set(BSD "FreeBSD")
set(CMAKE_DL_LIBS "")
set(CMAKE_C_COMPILE_OPTIONS_PIC "-fPIC")
set(CMAKE_C_COMPILE_OPTIONS_PIE "-fPIE")
# PIE link options are managed in Compiler/<compiler>.cmake file
set(CMAKE_SHARED_LIBRARY_C_FLAGS "-fPIC")            # -pic
set(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "-shared")       # -shared
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")         # +s, flag for exe link to use shared lib
set(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG "-Wl,-rpath,")       # -rpath
set(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG_SEP ":")   # : or empty
# Does not require -z origin since 10.2-RELEASE
set(CMAKE_SHARED_LIBRARY_RPATH_ORIGIN_TOKEN "\$ORIGIN")
set(CMAKE_SHARED_LIBRARY_RPATH_LINK_C_FLAG "-Wl,-rpath-link,")
set(CMAKE_SHARED_LIBRARY_SONAME_C_FLAG "-Wl,-soname,")
set(CMAKE_EXE_EXPORTS_C_FLAG "-Wl,--export-dynamic")

# Shared libraries with no builtin soname may not be linked safely by
# specifying the file path.
set(CMAKE_PLATFORM_USES_PATH_WHEN_NO_SONAME 1)

# Initialize C link type selection flags.  These flags are used when
# building a shared library, shared module, or executable that links
# to other libraries to select whether to use the static or shared
# versions of the libraries.
foreach(type SHARED_LIBRARY SHARED_MODULE EXE)
  set(CMAKE_${type}_LINK_STATIC_C_FLAGS "-Wl,-Bstatic")
  set(CMAKE_${type}_LINK_DYNAMIC_C_FLAGS "-Wl,-Bdynamic")
endforeach()


# Features for LINK_LIBRARY generator expression
## check linker capabilities
if(NOT DEFINED _CMAKE_LINKER_PUSHPOP_STATE_SUPPORTED)
  execute_process(COMMAND "${CMAKE_LINKER}" --help
                  OUTPUT_VARIABLE __linker_help
                  ERROR_VARIABLE __linker_help)
  if(__linker_help MATCHES "--push-state" AND __linker_help MATCHES "--pop-state")
    set(_CMAKE_LINKER_PUSHPOP_STATE_SUPPORTED TRUE CACHE INTERNAL "linker supports push/pop state")
  else()
    set(_CMAKE_LINKER_PUSHPOP_STATE_SUPPORTED FALSE CACHE INTERNAL "linker supports push/pop state")
  endif()
  unset(__linker_help)
endif()
## WHOLE_ARCHIVE: Force loading all members of an archive
if(_CMAKE_LINKER_PUSHPOP_STATE_SUPPORTED)
  set(CMAKE_LINK_LIBRARY_USING_WHOLE_ARCHIVE "LINKER:--push-state,--whole-archive"
                                             "<LINK_ITEM>"
                                             "LINKER:--pop-state")
else()
  set(CMAKE_LINK_LIBRARY_USING_WHOLE_ARCHIVE "LINKER:--whole-archive"
                                             "<LINK_ITEM>"
                                             "LINKER:--no-whole-archive")
endif()
set(CMAKE_LINK_LIBRARY_USING_WHOLE_ARCHIVE_SUPPORTED TRUE)


# Features for LINK_GROUP generator expression
## RESCAN: request the linker to rescan static libraries until there is
## no pending undefined symbols
set(CMAKE_LINK_GROUP_USING_RESCAN "LINKER:--start-group" "LINKER:--end-group")
set(CMAKE_LINK_GROUP_USING_RESCAN_SUPPORTED TRUE)


include(Platform/UnixPaths)
