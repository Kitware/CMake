set(QNXNTO 1)

set(CMAKE_DL_LIBS "")

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

include(Platform/GNU)
unset(CMAKE_LIBRARY_ARCHITECTURE_REGEX)

macro(__compiler_qcc lang)
  # http://www.qnx.com/developers/docs/6.4.0/neutrino/utilities/q/qcc.html#examples
  set(CMAKE_${lang}_COMPILE_OPTIONS_TARGET "-V")

  set(CMAKE_INCLUDE_SYSTEM_FLAG_${lang} "-Wp,-isystem,")
  set(CMAKE_DEPFILE_FLAGS_${lang} "-Wc,-MMD,<DEPFILE>,-MT,<OBJECT>,-MF,<DEPFILE>")

  if (lang STREQUAL CXX)
    # If the toolchain uses qcc for CMAKE_CXX_COMPILER instead of QCC, the
    # default for the driver is not c++.
    set(CMAKE_CXX_COMPILE_OBJECT
    "<CMAKE_CXX_COMPILER> -lang-c++ <DEFINES> <FLAGS> -o <OBJECT> -c <SOURCE>")
  endif()

endmacro()
