# CMake version number components.
set(CMake_VERSION_MAJOR 3)
set(CMake_VERSION_MINOR 15)
set(CMake_VERSION_PATCH 20190726)
#set(CMake_VERSION_RC 0)
set(CMake_VERSION_IS_DIRTY 0)

# Start with the full version number used in tags.  It has no dev info.
set(CMake_VERSION
  "${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}.${CMake_VERSION_PATCH}")
if(DEFINED CMake_VERSION_RC)
  set(CMake_VERSION "${CMake_VERSION}-rc${CMake_VERSION_RC}")
endif()

# Releases define a small patch level.
if("${CMake_VERSION_PATCH}" VERSION_LESS 20000000)
  set(CMake_VERSION_IS_RELEASE 1)
else()
  set(CMake_VERSION_IS_RELEASE 0)
endif()

if(EXISTS ${CMake_SOURCE_DIR}/.git)
  find_package(Git QUIET)
  if(GIT_FOUND)
    macro(_git)
      execute_process(
        COMMAND ${GIT_EXECUTABLE} ${ARGN}
        WORKING_DIRECTORY ${CMake_SOURCE_DIR}
        RESULT_VARIABLE _git_res
        OUTPUT_VARIABLE _git_out OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_VARIABLE _git_err ERROR_STRIP_TRAILING_WHITESPACE
        )
    endmacro()
  endif()
endif()

if(NOT CMake_VERSION_IS_RELEASE)
  # Try to identify the current development source version.
  if(COMMAND _git)
    _git(rev-parse --verify -q --short=4 HEAD)
    if(_git_out)
      set(CMake_VERSION "${CMake_VERSION}-g${_git_out}")
      _git(update-index -q --refresh)
      _git(diff-index --name-only HEAD --)
      if(_git_out)
        set(CMake_VERSION_IS_DIRTY 1)
      endif()
    endif()
  endif()
endif()

# Extract the version suffix component.
if(CMake_VERSION MATCHES "-(.*)$")
  set(CMake_VERSION_SUFFIX "${CMAKE_MATCH_1}")
else()
  set(CMake_VERSION_SUFFIX "")
endif()
if(CMake_VERSION_IS_DIRTY)
  set(CMake_VERSION ${CMake_VERSION}-dirty)
endif()
