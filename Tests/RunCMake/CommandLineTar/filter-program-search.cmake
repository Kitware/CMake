# Test that an external decompression filter program is not taken from the
# directory being extracted into, and that one installed in an absolute PATH
# entry is still used.
#
# The grzip filter is used because libarchive never implements it in-process,
# so it always goes through the external-program path regardless of how
# libarchive was built.  A copy of cmake itself stands in for the program.
#
# Which filter libarchive used is told by its own diagnostics rather than by
# anything the program prints, because the program's output is not observable
# here: libarchive gives the program's stdout to the decompression stream,
# and on Apple platforms it spawns the program with
# POSIX_SPAWN_CLOEXEC_DEFAULT and file actions for stdin and stdout only, so
# the program has no stderr either.  "Unrecognized archive format" means no
# filter bid on the archive at all, and "Child process exited" is reported
# only by the external-program filter, so it means a program was selected and
# run.
#
# Entries are prepended to the inherited PATH rather than replacing it,
# because cmake is linked against runtime libraries that are themselves
# found through PATH in some configurations, and so could not start at all
# without it.

if(CMAKE_HOST_WIN32)
  set(exe_suffix ".exe")
  set(path_sep ";")
else()
  set(exe_suffix "")
  set(path_sep ":")
endif()

set(inherited_path "$ENV{PATH}")

set(archive "${CMAKE_CURRENT_LIST_DIR}/filter-program-magic.grz")

set(extract_dir "${CMAKE_CURRENT_BINARY_DIR}/extract")
set(absolute_dir "${CMAKE_CURRENT_BINARY_DIR}/absolute")
set(relative_parent "${CMAKE_CURRENT_BINARY_DIR}/relative")
set(relative_dir "${relative_parent}/programs")

file(REMOVE_RECURSE "${extract_dir}" "${absolute_dir}" "${relative_parent}")
file(MAKE_DIRECTORY "${extract_dir}" "${absolute_dir}" "${relative_dir}")

# Place a copy of cmake where each case needs one.
function(place_program dir)
  set(program "${dir}/grzip${exe_suffix}")
  file(COPY_FILE "${CMAKE_COMMAND}" "${program}")
  if(NOT CMAKE_HOST_WIN32)
    file(CHMOD "${program}" PERMISSIONS
      OWNER_READ OWNER_WRITE OWNER_EXECUTE
      GROUP_READ GROUP_EXECUTE
      WORLD_READ WORLD_EXECUTE)
  endif()
endfunction()

# Run extraction in 'wd' with PATH set to 'path'.  Report whether libarchive
# ran a filter program, whether it found no filter at all, and what it said.
function(try_extract wd path out_ran out_unrecognized out_output)
  set(saved_path "$ENV{PATH}")
  set(ENV{PATH} "${path}")
  execute_process(
    COMMAND "${CMAKE_COMMAND}" -E tar xf "${archive}"
    WORKING_DIRECTORY "${wd}"
    OUTPUT_VARIABLE captured_stdout
    ERROR_VARIABLE captured_stderr
    RESULT_VARIABLE captured_result
    )
  set(ENV{PATH} "${saved_path}")
  set(output "${captured_stdout}${captured_stderr}")
  # libarchive reports each of the outcomes below, so no output at all means
  # cmake did not start.  Say so here rather than leave an assertion to fail
  # for a reason that has nothing to do with what it checks.
  if(output STREQUAL "")
    message(FATAL_ERROR
      "Extraction produced no output, so cmake did not run: "
      "result [${captured_result}]")
  endif()
  if(output MATCHES "Child process exited")
    set(${out_ran} 1 PARENT_SCOPE)
  else()
    set(${out_ran} 0 PARENT_SCOPE)
  endif()
  if(output MATCHES "Unrecognized archive format")
    set(${out_unrecognized} 1 PARENT_SCOPE)
  else()
    set(${out_unrecognized} 0 PARENT_SCOPE)
  endif()
  set(${out_output} "${output}" PARENT_SCOPE)
  # The archive holds nothing but a magic number, so extraction must fail
  # whether or not a filter program was available to run.
  if(captured_result STREQUAL "0")
    message(FATAL_ERROR "Extraction unexpectedly succeeded:\n${output}")
  endif()
endfunction()

# A grzip program installed on this machine would satisfy the lookup in the
# cases that expect no program to be available, so look for one first, with
# nothing planted anywhere.  That a program ran is the thing to look for
# here: whether the format is recognized says nothing about whether a
# program is installed, because libarchive registers the bidder either way.
try_extract("${extract_dir}" "${inherited_path}" ran unrecognized output)
if(ran)
  message(STATUS "A grzip program is installed; skipping the rest")
  return()
endif()

# 1. A program in the directory being extracted into must not be used.
place_program("${extract_dir}")
try_extract("${extract_dir}" "${inherited_path}" ran unrecognized output)
if(ran)
  message(FATAL_ERROR
    "A filter program in the extraction directory was executed:\n${output}")
endif()
if(NOT unrecognized)
  message(FATAL_ERROR
    "Expected no filter program to be available:\n${output}")
endif()

# 2. A program reachable only through a relative PATH entry must not be used,
#    because a relative entry is resolved against the working directory.
place_program("${relative_dir}")
try_extract("${relative_parent}" "programs${path_sep}${inherited_path}"
  ran unrecognized output)
if(ran)
  message(FATAL_ERROR
    "A filter program in a relative PATH entry was executed:\n${output}")
endif()
if(NOT unrecognized)
  message(FATAL_ERROR
    "Expected no filter program to be available:\n${output}")
endif()

# 3. A program installed in an absolute PATH entry must still be used, even
#    when another file of the same name sits in the extraction directory.
#    That libarchive recognizes the format is what shows the program was
#    selected.  Whether it then starts is libarchive's own business: it
#    starts the program with CreateProcessA(), so on Windows it cannot reach
#    one whose path the active code page cannot represent, as happens when
#    the build tree is under a non-ASCII path.
place_program("${absolute_dir}")
try_extract("${extract_dir}" "${absolute_dir}${path_sep}${inherited_path}"
  ran unrecognized output)
if(unrecognized)
  message(FATAL_ERROR
    "A filter program installed in an absolute PATH entry was not used:"
    "\n${output}")
endif()
