# Regression guard for C4: EMBTClang-FindBinUtils must pin the linker and the
# LLVM binutils to absolute paths beside the compiler, each a distinct tool.
# The distinctness check specifically catches the find_program cache-reuse trap
# -- dropping the per-iteration unset(... CACHE) collapses every tool onto the
# first (ld.lld).  Gated on bcc64x being present; a no-op where it is absent.

if(NOT WIN32)
  return()
endif()

set(_hints "")
if(DEFINED ENV{TP})
  list(APPEND _hints "$ENV{TP}/bin64")
endif()
find_program(EMBT_BCC64X NAMES bcc64x HINTS ${_hints})
if(NOT EMBT_BCC64X)
  message(STATUS "bcc64x not found on PATH or under $TP; binutils pinning check skipped.")
  return()
endif()

# Start from a clean nested build tree: compiler determination (and thus
# FindBinUtils) is cached, so a stale tree would not re-run the logic under test.
file(REMOVE_RECURSE "${CMAKE_CURRENT_BINARY_DIR}/build")
execute_process(
  COMMAND ${CMAKE_COMMAND}
    -S "${CMAKE_CURRENT_SOURCE_DIR}/binutils"
    -B "${CMAKE_CURRENT_BINARY_DIR}/build"
    -G "${CMAKE_GENERATOR}"
    "-DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}"
    "-DCMAKE_C_COMPILER=${EMBT_BCC64X}"
    -DCMAKE_C_COMPILER_WORKS=1
  OUTPUT_VARIABLE _out ERROR_VARIABLE _err)

# FindBinUtils pins tools found under REALPATH(compiler dir), so canonicalize
# both sides before comparing -- the work trees are reachable through junctions,
# where a literal directory would not string-match the resolved one.
get_filename_component(_bindir "${EMBT_BCC64X}" REALPATH)
get_filename_component(_bindir "${_bindir}" DIRECTORY)
set(_seen "")
foreach(_tool LINKER NM OBJDUMP STRIP DLLTOOL)
  if(NOT _out MATCHES "BINUTIL ${_tool}=\\[([^]]+)\\]")
    message(SEND_ERROR "no CMAKE_${_tool} reported.\n${_out}${_err}")
    continue()
  endif()
  set(_path "${CMAKE_MATCH_1}")
  if(NOT IS_ABSOLUTE "${_path}" OR NOT EXISTS "${_path}")
    message(SEND_ERROR "CMAKE_${_tool} is not an existing absolute path: [${_path}]")
  endif()
  get_filename_component(_dir "${_path}" REALPATH)
  get_filename_component(_dir "${_dir}" DIRECTORY)
  if(NOT _dir STREQUAL _bindir)
    message(SEND_ERROR "CMAKE_${_tool} is not beside the compiler: [${_path}]")
  endif()
  if(_path IN_LIST _seen)
    message(SEND_ERROR
      "CMAKE_${_tool} collapsed onto an already-pinned tool: [${_path}] "
      "(find_program cache-reuse regression)")
  endif()
  list(APPEND _seen "${_path}")
endforeach()
