set(CMAKE_FIND_USE_CMAKE_ENVIRONMENT_PATH OFF)
set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH OFF)
set(CMAKE_FIND_USE_CMAKE_SYSTEM_PATH OFF)
set(CMAKE_FIND_FRAMEWORK NEVER)

# FIXME: Avoid unchecked construction of TCLTK_POSSIBLE_BIN_PATHS
# and TCLTK_POSSIBLE_LIB_PATHS.  Fix FindTclsh and FindTCL later.
set(CMAKE_IGNORE_PATH /bin /lib)

function(check_layout name)
  cmake_parse_arguments(PARSE_ARGV 1 arg ""
    "ARCH;TCL_LIBRARY;TK_LIBRARY;INCLUDE_PATH" "PREFIXES;SUFFIXES;FILES")

  set(prefix "${CMAKE_CURRENT_BINARY_DIR}/${name}")
  file(REMOVE_RECURSE "${prefix}")
  foreach(f IN LISTS arg_FILES)
    get_filename_component(dir "${prefix}/${f}" DIRECTORY)
    file(MAKE_DIRECTORY "${dir}")
    file(WRITE "${prefix}/${f}" "")
  endforeach()

  set(CMAKE_FIND_LIBRARY_PREFIXES "${arg_PREFIXES}")
  set(CMAKE_FIND_LIBRARY_SUFFIXES "${arg_SUFFIXES}")
  set(CMAKE_LIBRARY_ARCHITECTURE "${arg_ARCH}")
  set(CMAKE_PREFIX_PATH "${prefix}")
  # Empty skips the host tclsh/wish search, and FindTclsh running tclsh
  set(TCL_TCLSH "")
  set(TK_WISH "")

  find_package(TCL QUIET)

  set(TCL_INCLUDE_PATH_expect "${arg_INCLUDE_PATH}")
  set(TK_INCLUDE_PATH_expect "${arg_INCLUDE_PATH}")
  set(TCL_LIBRARY_expect "${arg_TCL_LIBRARY}")
  set(TK_LIBRARY_expect "${arg_TK_LIBRARY}")
  foreach(var IN ITEMS TCL_LIBRARY TK_LIBRARY TCL_INCLUDE_PATH TK_INCLUDE_PATH)
    if(NOT "${${var}}" STREQUAL "${prefix}/${${var}_expect}")
      message(SEND_ERROR "${name}: ${var} is\n  ${${var}}\n"
        "but expected\n  ${prefix}/${${var}_expect}")
    endif()
    unset(${var} CACHE)
  endforeach()
endfunction()

check_layout(Tk9-nmake
  PREFIXES "" SUFFIXES .lib
  FILES include/tcl.h include/tk.h lib/tcl90.lib lib/tcl9tk90.lib
  TCL_LIBRARY lib/tcl90.lib TK_LIBRARY lib/tcl9tk90.lib INCLUDE_PATH include
  )

check_layout(Tk9-mingw
  PREFIXES lib "" SUFFIXES .dll.a .a .lib
  FILES include/tcl.h include/tk.h lib/libtcl90.dll.a lib/libtcl9tk90.dll.a
  TCL_LIBRARY lib/libtcl90.dll.a TK_LIBRARY lib/libtcl9tk90.dll.a
  INCLUDE_PATH include
  )

check_layout(Tk9-unix
  PREFIXES lib SUFFIXES .so
  FILES include/tcl.h include/tk.h lib/libtcl9.0.so lib/libtcl9tk9.0.so
  TCL_LIBRARY lib/libtcl9.0.so TK_LIBRARY lib/libtcl9tk9.0.so
  INCLUDE_PATH include
  )

check_layout(Tk9-homebrew
  PREFIXES lib SUFFIXES .dylib
  FILES include/tcl-tk/tcl.h include/tcl-tk/tk.h
        lib/libtcl9.0.dylib lib/libtcl9tk9.0.dylib
  TCL_LIBRARY lib/libtcl9.0.dylib TK_LIBRARY lib/libtcl9tk9.0.dylib
  INCLUDE_PATH include/tcl-tk
  )

check_layout(Tk86-Tk9-nmake
  PREFIXES "" SUFFIXES .lib
  FILES include/tcl.h include/tk.h lib/tcl86t.lib lib/tk86t.lib
        lib/tcl90.lib lib/tcl9tk90.lib
  TCL_LIBRARY lib/tcl90.lib TK_LIBRARY lib/tcl9tk90.lib INCLUDE_PATH include
  )

check_layout(Tk86-Tk9-unix
  PREFIXES lib SUFFIXES .so
  FILES include/tcl8.6/tcl.h include/tcl8.6/tk.h
        include/tcl9.0/tcl.h include/tcl9.0/tk.h
        lib/libtcl8.6.so lib/libtk8.6.so lib/libtcl9.0.so lib/libtcl9tk9.0.so
  TCL_LIBRARY lib/libtcl9.0.so TK_LIBRARY lib/libtcl9tk9.0.so
  INCLUDE_PATH include/tcl9.0
  )

check_layout(Tk86-nmake
  PREFIXES "" SUFFIXES .lib
  FILES include/tcl.h include/tk.h lib/tcl86t.lib lib/tk86t.lib
  TCL_LIBRARY lib/tcl86t.lib TK_LIBRARY lib/tk86t.lib INCLUDE_PATH include
  )

check_layout(Tk86-debian
  ARCH x86_64-linux-gnu PREFIXES lib SUFFIXES .so
  FILES include/tcl8.6/tcl.h include/tcl8.6/tk.h
        lib/x86_64-linux-gnu/libtcl8.6.so lib/x86_64-linux-gnu/libtk8.6.so
  TCL_LIBRARY lib/x86_64-linux-gnu/libtcl8.6.so
  TK_LIBRARY lib/x86_64-linux-gnu/libtk8.6.so
  INCLUDE_PATH include/tcl8.6
  )

# Distro default links (libtcl.so, libtk.so, include/tcl) still win.
check_layout(Tk86-Tk9-debian
  ARCH x86_64-linux-gnu PREFIXES lib SUFFIXES .so
  FILES include/tcl/tcl.h include/tcl/tk.h
        include/tcl8.6/tcl.h include/tcl8.6/tk.h
        include/tcl9.0/tcl.h include/tcl9.0/tk.h
        lib/x86_64-linux-gnu/libtcl.so lib/x86_64-linux-gnu/libtk.so
        lib/x86_64-linux-gnu/libtcl8.6.so lib/x86_64-linux-gnu/libtk8.6.so
        lib/x86_64-linux-gnu/libtcl9.0.so lib/x86_64-linux-gnu/libtcl9tk9.0.so
  TCL_LIBRARY lib/x86_64-linux-gnu/libtcl.so
  TK_LIBRARY lib/x86_64-linux-gnu/libtk.so
  INCLUDE_PATH include/tcl
  )
