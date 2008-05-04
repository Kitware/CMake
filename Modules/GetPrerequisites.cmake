# GetPrerequisites.cmake
#
# This script provides functions to list the .dll, .dylib or .so files that an
# executable or shared library file depends on. (Its prerequisites.)
#
# It uses various tools to obtain the list of required shared library files:
#   dumpbin (Windows)
#   ldd (Linux/Unix)
#   otool (Mac OSX)
#
# The following functions are provided by this script:
#   gp_append_unique
#   gp_file_type
#   is_file_executable
#   get_prerequisites
#   list_prerequisites
#
# Requires CMake 2.5 or greater because it uses function, break, return and
# PARENT_SCOPE.
#
cmake_minimum_required(VERSION 2.5 FATAL_ERROR)


# gp_append_unique list_var value
#
# Append value to the list variable ${list_var} only if the value is not
# already in the list.
#
function(gp_append_unique list_var value)
  set(contains 0)

  foreach(item ${${list_var}})
    if("${item}" STREQUAL "${value}")
      set(contains 1)
      break()
    endif("${item}" STREQUAL "${value}")
  endforeach(item)

  if(NOT contains)
    set(${list_var} ${${list_var}} "${value}" PARENT_SCOPE)
  endif(NOT contains)
endfunction(gp_append_unique)


# gp_file_type original_file file type_var
#
# Return the type of ${file} with respect to ${original_file}. String
# describing type of prerequisite is returned in variable named ${type_var}.
#
# Possible types are:
#   system
#   local
#   embedded
#   other
#
function(gp_file_type original_file file type_var)
  set(is_embedded 0)
  set(is_local 0)
  set(is_system 0)

  string(TOLOWER "${original_file}" original_lower)
  string(TOLOWER "${file}" lower)

  if("${file}" MATCHES "^@(executable|loader)_path")
    set(is_embedded 1)
  endif("${file}" MATCHES "^@(executable|loader)_path")

  if(NOT is_embedded)
    if("${file}" MATCHES "^(/System/Library/|/usr/lib/)")
      set(is_system 1)
    endif("${file}" MATCHES "^(/System/Library/|/usr/lib/)")

    if(WIN32)
      string(TOLOWER "$ENV{SystemRoot}" sysroot)
      string(REGEX REPLACE "\\\\" "/" sysroot "${sysroot}")

      string(TOLOWER "$ENV{windir}" windir)
      string(REGEX REPLACE "\\\\" "/" windir "${windir}")

      if("${lower}" MATCHES "^(${sysroot}/system|${windir}/system|msvc[^/]+dll)")
        set(is_system 1)
      endif("${lower}" MATCHES "^(${sysroot}/system|${windir}/system|msvc[^/]+dll)")
    endif(WIN32)

    if(NOT is_system)
      get_filename_component(original_path "${original_lower}" PATH)
      get_filename_component(path "${lower}" PATH)
      if("${original_path}" STREQUAL "${path}")
        set(is_local 1)
      endif("${original_path}" STREQUAL "${path}")
    endif(NOT is_system)
  endif(NOT is_embedded)

  # Return type string based on computed booleans:
  #
  set(type "other")

  if(is_system)
    set(type "system")
  else(is_system)
    if(is_embedded)
      set(type "embedded")
    else(is_embedded)
      if(is_local)
        set(type "local")
      endif(is_local)
    endif(is_embedded)
  endif(is_system)

  set(${type_var} "${type}" PARENT_SCOPE)
endfunction(gp_file_type)


# is_file_executable file result_var
#
# Return 1 in ${result_var} if ${file} is a binary executable.
#
# Return 0 in ${result_var} otherwise.
#
function(is_file_executable file result_var)
  #
  # A file is not executable until proven otherwise:
  #
  set(${result_var} 0 PARENT_SCOPE)

  get_filename_component(file_full "${file}" ABSOLUTE)
  string(TOLOWER "${file_full}" file_full_lower)

  # If file name ends in .exe or .dll on Windows, *assume* executable:
  #
  if(WIN32)
    if("${file_full_lower}" MATCHES "\\.(exe|dll)$")
      set(${result_var} 1 PARENT_SCOPE)
      return()
    endif("${file_full_lower}" MATCHES "\\.(exe|dll)$")

    # A clause could be added here that uses output or return value of dumpbin
    # to determine ${result_var}. In 95%+ practical cases, the exe|dll name
    # match will be sufficient...
    #
  endif(WIN32)

  # Use the information returned from the Unix shell command "file" to
  # determine if ${file_full} should be considered an executable file...
  #
  # If the file command's output contains "executable" and does *not* contain
  # "text" then it is likely an executable suitable for prerequisite analysis
  # via the get_prerequisites macro.
  #
  if(UNIX)
    if(NOT file_cmd)
      find_program(file_cmd "file")
    endif(NOT file_cmd)

    if(file_cmd)
      execute_process(COMMAND "${file_cmd}" "${file_full}"
        OUTPUT_VARIABLE file_ov
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )

      # Replace the name of the file in the output with a placeholder token
      # (the string " _file_full_ ") so that just in case the path name of
      # the file contains the word "text" or "executable" we are not fooled
      # into thinking "the wrong thing" because the file name matches the
      # other 'file' command output we are looking for...
      #
      string(REPLACE "${file_full}" " _file_full_ " file_ov "${file_ov}")
      string(TOLOWER "${file_ov}" file_ov)

      #message(STATUS "file_ov='${file_ov}'")
      if("${file_ov}" MATCHES "executable")
        #message(STATUS "executable!")
        if("${file_ov}" MATCHES "text")
          #message(STATUS "but text, so *not* a binary executable!")
        else("${file_ov}" MATCHES "text")
          set(${result_var} 1 PARENT_SCOPE)
          return()
        endif("${file_ov}" MATCHES "text")
      endif("${file_ov}" MATCHES "executable")
    else(file_cmd)
      message(STATUS "warning: No 'file' command, skipping execute_process...")
    endif(file_cmd)
  endif(UNIX)
endfunction(is_file_executable)


# get_prerequisites target prerequisites_var exclude_system recurse
#
# Get the list of shared library files required by ${target}. The list in
# the variable named ${prerequisites_var} should be empty on first entry to
# this function. On exit, ${prerequisites_var} will contain the list of
# required shared library files.
#
#  target is the full path to an executable file
#
#  prerequisites_var is the name of a CMake variable to contain the results
#
#  exclude_system is 0 or 1: 0 to include "system" prerequisites , 1 to
#   exclude them
#
#  recurse is 0 or 1: 0 for direct prerequisites only, 1 for all prerequisites
#   recursively
#
#  optional ARGV4 (verbose) is 0 or 1: 0 to skip informational message output,
#   1 to print it
#
function(get_prerequisites target prerequisites_var exclude_system recurse)
#  set(verbose 0)
#  if(NOT "${ARGV4}" STREQUAL "")
#    message(STATUS "ARGV4='${ARGV4}'")
#    set(verbose "${ARGV4}")
#  endif(NOT "${ARGV4}" STREQUAL "")
#  message(STATUS "verbose='${verbose}'")
  set(verbose 0)

  set(eol_char "E")

  # <setup-gp_tool-vars>
  #
  # Try to choose the right tool by default. Caller can set gp_tool prior to
  # calling this function to force using a different tool.
  #
  if("${gp_tool}" STREQUAL "")
    set(gp_tool "ldd")
    if(APPLE)
      set(gp_tool "otool")
    endif(APPLE)
    if(WIN32)
      set(gp_tool "dumpbin")
    endif(WIN32)
  endif("${gp_tool}" STREQUAL "")

  set(gp_tool_known 0)

  if("${gp_tool}" STREQUAL "ldd")
    set(gp_cmd_args "")
    set(gp_regex "^\t([\t ]+)[\t ].*${eol_char}$")
    set(gp_regex_cmp_count 1)
    set(gp_tool_known 1)
  endif("${gp_tool}" STREQUAL "ldd")

  if("${gp_tool}" STREQUAL "otool")
    set(gp_cmd_args "-L")
    set(gp_regex "^\t([^\t]+) \\(compatibility version ([0-9]+.[0-9]+.[0-9]+), current version ([0-9]+.[0-9]+.[0-9]+)\\)${eol_char}$")
    set(gp_regex_cmp_count 3)
    set(gp_tool_known 1)
  endif("${gp_tool}" STREQUAL "otool")

  if("${gp_tool}" STREQUAL "dumpbin")
    set(gp_cmd_args "/dependents")
    set(gp_regex "^    ([^ ].*[Dd][Ll][Ll])${eol_char}$")
    set(gp_regex_cmp_count 1)
    set(gp_tool_known 1)
    set(ENV{VS_UNICODE_OUTPUT} "") # Block extra output from inside VS IDE.
  endif("${gp_tool}" STREQUAL "dumpbin")

  if(NOT gp_tool_known)
    message(STATUS "warning: gp_tool='${gp_tool}' is an unknown tool...")
    message(STATUS "CMake function get_prerequisites needs more code to handle '${gp_tool}'")
    message(STATUS "Valid gp_tool values are dumpbin, ldd and otool.")
    return()
  endif(NOT gp_tool_known)

  set(gp_cmd_paths ${gp_cmd_paths}
    "C:/Program Files/Microsoft Visual Studio 9.0/VC/bin"
    "C:/Program Files (x86)/Microsoft Visual Studio 9.0/VC/bin"
    "C:/Program Files/Microsoft Visual Studio 8/VC/BIN"
    "C:/Program Files (x86)/Microsoft Visual Studio 8/VC/BIN"
    "C:/Program Files/Microsoft Visual Studio .NET 2003/VC7/BIN"
    "C:/Program Files (x86)/Microsoft Visual Studio .NET 2003/VC7/BIN"
    "/usr/local/bin"
    "/usr/bin"
    )

  find_program(gp_cmd ${gp_tool} PATHS ${gp_cmd_paths})

  if(NOT gp_cmd)
    message(STATUS "warning: could not find '${gp_tool}' - cannot analyze prerequisites...")
    return()
  endif(NOT gp_cmd)

  if("${gp_tool}" STREQUAL "dumpbin")
    # When running dumpbin, it also needs the "Common7/IDE" directory in the
    # PATH. It will already be in the PATH if being run from a Visual Studio
    # command prompt. Add it to the PATH here in case we are running from a
    # different command prompt.
    #
    get_filename_component(gp_cmd_dir "${gp_cmd}" PATH)
    get_filename_component(gp_cmd_dlls_dir "${gp_cmd_dir}/../../Common7/IDE" ABSOLUTE)
    if(EXISTS "${gp_cmd_dlls_dir}")
      set(ENV{PATH} "$ENV{PATH};${gp_cmd_dlls_dir}")
    endif(EXISTS "${gp_cmd_dlls_dir}")
  endif("${gp_tool}" STREQUAL "dumpbin")
  #
  # </setup-gp_tool-vars>

  # Track new prerequisites at each new level of recursion. Start with an
  # empty list at each level:
  #
  set(unseen_prereqs)

  # Run gp_cmd on the target:
  #
  execute_process(
    COMMAND ${gp_cmd} ${gp_cmd_args} ${target}
    OUTPUT_VARIABLE gp_cmd_ov
    )

  if(verbose)
    message(STATUS "<RawOutput cmd='${gp_cmd} ${gp_cmd_args} ${target}'>")
    message(STATUS "gp_cmd_ov='${gp_cmd_ov}'")
    message(STATUS "</RawOutput>")
  endif(verbose)

  get_filename_component(target_dir "${target}" PATH)

  # Convert to a list of lines:
  #
  string(REGEX REPLACE ";" "\\\\;" candidates "${gp_cmd_ov}")
  string(REGEX REPLACE "\n" "${eol_char};" candidates "${candidates}")

  # Analyze each line for file names that match the regular expression:
  #
  foreach(candidate ${candidates})
  if("${candidate}" MATCHES "${gp_regex}")
    # Extract information from each candidate:
    string(REGEX REPLACE "${gp_regex}" "\\1" raw_item "${candidate}")

    if(gp_regex_cmp_count GREATER 1)
      string(REGEX REPLACE "${gp_regex}" "\\2" raw_compat_version "${candidate}")
      string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$" "\\1" compat_major_version "${raw_compat_version}")
      string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$" "\\2" compat_minor_version "${raw_compat_version}")
      string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$" "\\3" compat_patch_version "${raw_compat_version}")
    endif(gp_regex_cmp_count GREATER 1)

    if(gp_regex_cmp_count GREATER 2)
      string(REGEX REPLACE "${gp_regex}" "\\3" raw_current_version "${candidate}")
      string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$" "\\1" current_major_version "${raw_current_version}")
      string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$" "\\2" current_minor_version "${raw_current_version}")
      string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$" "\\3" current_patch_version "${raw_current_version}")
    endif(gp_regex_cmp_count GREATER 2)

    # Using find_program on Windows will find dll files that are in the PATH.
    # (Converting simple file names into full path names if found.)
    #
    set(item "item-NOTFOUND")
    find_program(item "${raw_item}" PATHS "${target_dir}")
    if(NOT item)
      set(item "${raw_item}")
    endif(NOT item)

    if(verbose)
      message(STATUS "raw_item='${raw_item}'")
      message(STATUS "item='${item}'")
    endif(verbose)

    # Add each item unless it is excluded:
    #
    set(add_item 1)

    if(${exclude_system})
      set(type "")
      gp_file_type("${target}" "${item}" type)

      if("${type}" STREQUAL "system")
        set(add_item 0)
      endif("${type}" STREQUAL "system")
    endif(${exclude_system})

    if(add_item)
      list(LENGTH ${prerequisites_var} list_length_before_append)
      gp_append_unique(${prerequisites_var} "${item}")
      list(LENGTH ${prerequisites_var} list_length_after_append)

      if(${recurse})
        # If item was really added, this is the first time we have seen it.
        # Add it to unseen_prereqs so that we can recursively add *its*
        # prerequisites...
        #
        if(NOT list_length_before_append EQUAL list_length_after_append)
          set(unseen_prereqs ${unseen_prereqs} "${item}")
        endif(NOT list_length_before_append EQUAL list_length_after_append)
      endif(${recurse})
    endif(add_item)
  else("${candidate}" MATCHES "${gp_regex}")
    if(verbose)
      message(STATUS "ignoring non-matching line: '${candidate}'")
    endif(verbose)
  endif("${candidate}" MATCHES "${gp_regex}")
  endforeach(candidate)

  list(SORT ${prerequisites_var})

  if(${recurse})
    set(more_inputs ${unseen_prereqs})
    foreach(input ${more_inputs})
      get_prerequisites("${input}" ${prerequisites_var} ${exclude_system} ${recurse})
    endforeach(input)
  endif(${recurse})

  set(${prerequisites_var} ${${prerequisites_var}} PARENT_SCOPE)
endfunction(get_prerequisites)


# list_prerequisites target all exclude_system verbose
#
#  ARGV0 (target) is the full path to an executable file
#
#  optional ARGV1 (all) is 0 or 1: 0 for direct prerequisites only,
#   1 for all prerequisites recursively
#
#  optional ARGV2 (exclude_system) is 0 or 1: 0 to include "system"
#   prerequisites , 1 to exclude them
#
#  optional ARGV3 (verbose) is 0 or 1: 0 to print only full path
#   names of prerequisites, 1 to print extra information
#
function(list_prerequisites target)
  if("${ARGV1}" STREQUAL "")
    set(all 1)
  else("${ARGV1}" STREQUAL "")
    set(all "${ARGV1}")
  endif("${ARGV1}" STREQUAL "")

  if("${ARGV2}" STREQUAL "")
    set(exclude_system 0)
  else("${ARGV2}" STREQUAL "")
    set(exclude_system "${ARGV2}")
  endif("${ARGV2}" STREQUAL "")

  if("${ARGV3}" STREQUAL "")
    set(verbose 0)
  else("${ARGV3}" STREQUAL "")
    set(verbose "${ARGV3}")
  endif("${ARGV3}" STREQUAL "")

  set(count 0)
  set(count_str "")
  set(print_count "${verbose}")
  set(print_prerequisite_type "${verbose}")
  set(print_target "${verbose}")
  set(type_str "")

  set(prereqs "")
  get_prerequisites("${target}" prereqs ${exclude_system} ${all})

  if(print_target)
    message(STATUS "File '${target}' depends on:")
  endif(print_target)

  foreach(d ${prereqs})
    math(EXPR count "${count} + 1")

    if(print_count)
      set(count_str "${count}. ")
    endif(print_count)

    if(print_prerequisite_type)
      gp_file_type("${target}" "${d}" type)
      set(type_str " (${type})")
    endif(print_prerequisite_type)

    message(STATUS "${count_str}${d}${type_str}")
  endforeach(d)
endfunction(list_prerequisites)


# list_prerequisites_by_glob glob_arg glob_exp
#
#  glob_arg is GLOB or GLOB_RECURSE
#
#  glob_exp is a globbing expression used with "file(GLOB" to retrieve a list
#   of matching files. If a matching file is executable, its prerequisites are
#   listed.
#
# Any additional (optional) arguments provided are passed along as the
# optional arguments to the list_prerequisites calls.
#
function(list_prerequisites_by_glob glob_arg glob_exp)
  message(STATUS "=============================================================================")
  message(STATUS "List prerequisites of executables matching ${glob_arg} '${glob_exp}'")
  message(STATUS "")
  file(${glob_arg} file_list ${glob_exp})
  foreach(f ${file_list})
    is_file_executable("${f}" is_f_executable)
    if(is_f_executable)
      message(STATUS "=============================================================================")
      list_prerequisites("${f}" ${ARGN})
      message(STATUS "")
    endif(is_f_executable)
  endforeach(f)
endfunction(list_prerequisites_by_glob)
