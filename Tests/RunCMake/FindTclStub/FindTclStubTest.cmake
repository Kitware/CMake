# Ignore default and hard-coded paths to avoid finding a system Tcl
set(CMAKE_FIND_USE_CMAKE_ENVIRONMENT_PATH OFF)
set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH OFF)
set(CMAKE_FIND_USE_CMAKE_SYSTEM_PATH OFF)
set(CMAKE_FIND_FRAMEWORK NEVER)
set(CMAKE_IGNORE_PATH
  "C:/Tcl/lib" "C:/Program Files/Tcl/lib" "$ENV{ProgramFiles}/Tcl/Lib")
set(CMAKE_FIND_LIBRARY_PREFIXES "")
set(CMAKE_FIND_LIBRARY_SUFFIXES .lib)

function(check_layout name)
  cmake_parse_arguments(PARSE_ARGV 1 arg ""
    "FOUND;TCL_STUB_LIBRARY;TK_STUB_LIBRARY;TTK_STUB_LIBRARY" "FILES")

  set(prefix "${CMAKE_CURRENT_BINARY_DIR}/${name}")
  file(REMOVE_RECURSE "${prefix}")
  foreach(f IN LISTS arg_FILES)
    get_filename_component(dir "${prefix}/${f}" DIRECTORY)
    file(MAKE_DIRECTORY "${dir}")
    file(WRITE "${prefix}/${f}" "")
  endforeach()

  set(CMAKE_PREFIX_PATH "${prefix}")
  # Empty skips the host tclsh/wish search, and FindTclsh running tclsh
  set(TCL_TCLSH "")
  set(TK_WISH "")

  find_package(TclStub QUIET)

  if(NOT TclStub_FOUND STREQUAL arg_FOUND)
    message(SEND_ERROR "${name}: TclStub_FOUND is '${TclStub_FOUND}'"
      " but expected '${arg_FOUND}'")
  endif()
  foreach(var IN ITEMS TCL_STUB_LIBRARY TK_STUB_LIBRARY TTK_STUB_LIBRARY)
    if(arg_${var})
      set(expect "${prefix}/${arg_${var}}")
    else()
      set(expect "${var}-NOTFOUND")
    endif()
    if(NOT "${${var}}" STREQUAL "${expect}")
      message(SEND_ERROR "${name}: ${var} is\n  ${${var}}\n"
        "but expected\n  ${expect}")
    endif()
  endforeach()
  foreach(var IN ITEMS TCL_STUB_LIBRARY TK_STUB_LIBRARY TTK_STUB_LIBRARY
                       TCL_LIBRARY TK_LIBRARY TCL_INCLUDE_PATH TK_INCLUDE_PATH)
    unset(${var} CACHE)
  endforeach()
endfunction()

check_layout(Stub86
  FILES include/tcl.h include/tk.h lib/tcl86t.lib lib/tk86t.lib
        lib/tclstub86.lib lib/tkstub86.lib
  FOUND TRUE
  TCL_STUB_LIBRARY lib/tclstub86.lib TK_STUB_LIBRARY lib/tkstub86.lib
  )

check_layout(Stub9
  FILES include/tcl.h include/tk.h lib/tcl90.lib lib/tcl9tk90.lib
        lib/tclstub.lib lib/tkstub.lib
  FOUND TRUE
  TCL_STUB_LIBRARY lib/tclstub.lib TK_STUB_LIBRARY lib/tkstub.lib
  )

check_layout(StubTclOnly
  FILES include/tcl.h lib/tcl86t.lib lib/tclstub86.lib
  FOUND TRUE
  TCL_STUB_LIBRARY lib/tclstub86.lib
  )

check_layout(StubTtk
  FILES include/tcl.h include/tk.h lib/tcl86t.lib lib/tk86t.lib
        lib/tclstub86.lib lib/tkstub86.lib lib/ttkstub86.lib
  FOUND TRUE
  TCL_STUB_LIBRARY lib/tclstub86.lib TK_STUB_LIBRARY lib/tkstub86.lib
  TTK_STUB_LIBRARY lib/ttkstub86.lib
  )

check_layout(NoStub
  FILES include/tcl.h include/tk.h lib/tcl86t.lib lib/tk86t.lib
  FOUND FALSE
  )
