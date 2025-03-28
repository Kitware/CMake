# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.


# This module is shared by multiple languages; use include blocker.
include_guard()

block(SCOPE_FOR POLICIES)
cmake_policy(SET CMP0054 NEW)

# WHOLE_ARCHIVE Feature for LINK_LIBRARY generator expression
## check linker capabilities
function(__cmake_set_whole_archive_feature __linker)
  unset(__lang)
  if(ARGC EQUAL "2")
    set(__lang "${ARGV1}_")
  endif()

  if(NOT __linker)
    set(_CMAKE_${__lang}LINKER_PUSHPOP_STATE_SUPPORTED FALSE CACHE INTERNAL "linker supports push/pop state")
  endif()

  if(NOT DEFINED _CMAKE_${__lang}LINKER_PUSHPOP_STATE_SUPPORTED)
    execute_process(COMMAND "${__linker}" --help
                    OUTPUT_VARIABLE __linker_help
                    ERROR_VARIABLE __linker_help)
    if(__linker_help MATCHES "--push-state" AND __linker_help MATCHES "--pop-state")
      set(_CMAKE_${__lang}LINKER_PUSHPOP_STATE_SUPPORTED TRUE CACHE INTERNAL "linker supports push/pop state")
    else()
      set(_CMAKE_${__lang}LINKER_PUSHPOP_STATE_SUPPORTED FALSE CACHE INTERNAL "linker supports push/pop state")
    endif()
  endif()
  ## WHOLE_ARCHIVE: Force loading all members of an archive
  if(_CMAKE_${__lang}LINKER_PUSHPOP_STATE_SUPPORTED)
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
endfunction()


## Configure system linker
__cmake_set_whole_archive_feature("${CMAKE_LINKER}")

endblock()
