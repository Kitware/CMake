# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.


# This module is shared by multiple languages; use include blocker.
include_guard()

# Probe whether the linker supports --push-state/--pop-state and cache the result
# in CMAKE_[<LANG>_]LINKER_PUSHPOP_STATE_SUPPORTED.
function(__cmake_check_linker_pushpop_state __linker __linker_options)
  unset(__lang)
  if(ARGC EQUAL "3")
    set(__lang "${ARGV2}_")
  endif()

  if(DEFINED CMAKE_${__lang}LINKER_PUSHPOP_STATE_SUPPORTED)
    return()
  endif()

  if(NOT __linker)
    set(CMAKE_${__lang}LINKER_PUSHPOP_STATE_SUPPORTED FALSE PARENT_SCOPE)
    return()
  endif()

  execute_process(COMMAND "${__linker}" ${__linker_options} --push-state --pop-state
                  OUTPUT_VARIABLE __linker_log
                  ERROR_VARIABLE __linker_log
                  COMMAND_ERROR_IS_FATAL NONE)
  if(__linker_log MATCHES "--push-state" OR __linker_log MATCHES "--pop-state")
    set(CMAKE_${__lang}LINKER_PUSHPOP_STATE_SUPPORTED FALSE PARENT_SCOPE)
  else()
    set(CMAKE_${__lang}LINKER_PUSHPOP_STATE_SUPPORTED TRUE PARENT_SCOPE)
  endif()
endfunction()

# WHOLE_ARCHIVE Feature for LINK_LIBRARY generator expression
function(__cmake_set_whole_archive_feature __linker __linker_options)
  unset(__lang)
  if(ARGC EQUAL "3")
    set(__lang "${ARGV2}_")
  endif()

  __cmake_check_linker_pushpop_state("${__linker}" "${__linker_options}" ${ARGV2})

  ## WHOLE_ARCHIVE: Force loading all members of an archive
  if(CMAKE_${__lang}LINKER_PUSHPOP_STATE_SUPPORTED)
    set(CMAKE_${__lang}LINK_LIBRARY_USING_WHOLE_ARCHIVE "LINKER:--push-state,--whole-archive"
                                                        "<LINK_ITEM>"
                                                        "LINKER:--pop-state" PARENT_SCOPE)
  else()
    set(CMAKE_${__lang}LINK_LIBRARY_USING_WHOLE_ARCHIVE "LINKER:--whole-archive"
                                                        "<LINK_ITEM>"
                                                        "LINKER:--no-whole-archive" PARENT_SCOPE)
  endif()
  set(CMAKE_${__lang}LINK_LIBRARY_USING_WHOLE_ARCHIVE_SUPPORTED TRUE PARENT_SCOPE)
  set(CMAKE_${__lang}LINK_LIBRARY_WHOLE_ARCHIVE_ATTRIBUTES LIBRARY_TYPE=STATIC DEDUPLICATION=YES OVERRIDE=DEFAULT PARENT_SCOPE)
  set(CMAKE_${__lang}LINKER_PUSHPOP_STATE_SUPPORTED "${CMAKE_${__lang}LINKER_PUSHPOP_STATE_SUPPORTED}" PARENT_SCOPE)
endfunction()


## Configure system linker
__cmake_set_whole_archive_feature("${CMAKE_LINKER}" "")
