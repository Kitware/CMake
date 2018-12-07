# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

# This is used internally by CMake and should not be included by user code.

# helper function that parses implicit include dirs from a single line
# for compilers that report them that way.  on success we return the
# list of dirs in id_var and set state_var to the 'done' state.
function(cmake_parse_implicit_include_line line lang id_var log_var state_var)
  # clear variables we append to (avoids possible polution from parent scopes)
  unset(rv)
  set(log "")

  # ccfe: cray compiler front end (PrgEnv-cray)
  if("${CMAKE_${lang}_COMPILER_ID}" STREQUAL "Cray" AND
     "${line}" MATCHES "-isystem")
    string(REGEX MATCHALL " (-I ?|-isystem )([^ ]*)" incs "${line}")
    foreach(inc IN LISTS incs)
      string(REGEX REPLACE " (-I ?|-isystem )([^ ]*)" "\\2" idir "${inc}")
      list(APPEND rv "${idir}")
    endforeach()
    if(rv)
      string(APPEND log "  got implicit includes via cray ccfe parser!\n")
    else()
      string(APPEND log "  warning: cray ccfe parse failed!\n")
    endif()
  endif()

  if(log)
    set(${log_var} "${log}" PARENT_SCOPE)
  endif()
  if(rv)
    set(${id_var} "${rv}" PARENT_SCOPE)
    set(${state_var} "done" PARENT_SCOPE)
  endif()
endfunction()

# top-level function to parse implicit include directory information
# from verbose compiler output. sets state_var in parent to 'done' on success.
function(cmake_parse_implicit_include_info text lang dir_var log_var state_var)
  set(state start)    # values: start, loading, done

  # clear variables we append to (avoids possible polution from parent scopes)
  set(implicit_dirs_tmp)
  set(log "")

  # go through each line of output...
  string(REGEX REPLACE "\r?\n" ";" output_lines "${text}")
  foreach(line IN LISTS output_lines)
    if(state STREQUAL start)
      string(FIND "${line}" "#include <...> search starts here:" rv)
      if(rv GREATER -1)
        set(state loading)
        string(APPEND log "  found start of implicit include info\n")
      else()
        cmake_parse_implicit_include_line("${line}" "${lang}" implicit_dirs_tmp
                                          linelog state)
        if(linelog)
          string(APPEND log ${linelog})
        endif()
        if(state STREQUAL done)
          break()
        endif()
      endif()
    elseif(state STREQUAL loading)
      string(FIND "${line}" "End of search list." rv)
      if(rv GREATER -1)
        set(state done)
        string(APPEND log "  end of search list found\n")
        break()
      else()
        string(STRIP "${line}" path)     # remove leading/trailing spaces
        if ("${path}" MATCHES " \\(framework directory\\)$")
          continue() # frameworks are handled elsewhere, ignore them here
        endif()
        string(REPLACE "\\" "/" path "${path}")
        list(APPEND implicit_dirs_tmp "${path}")
        string(APPEND log "    add: [${path}]\n")
      endif()
    endif()
  endforeach()

  # Log results.
  if(state STREQUAL done)
    string(APPEND log "  implicit include dirs: [${implicit_dirs_tmp}]\n")
  else()
    string(APPEND log "  warn: unable to parse implicit include dirs!\n")
  endif()

  # Return results.
  set(${dir_var} "${implicit_dirs_tmp}" PARENT_SCOPE)
  set(${log_var} "${log}" PARENT_SCOPE)
  set(${state_var} "${state}" PARENT_SCOPE)

endfunction()
