# CMake version number components.
set(CMake_VERSION_MAJOR 3)
set(CMake_VERSION_MINOR 15)
set(CMake_VERSION_PATCH 20190726)
#set(CMake_VERSION_RC 0)

# Releases define a small patch level.
if("${CMake_VERSION_PATCH}" VERSION_LESS 20000000)
  set(CMake_VERSION_IS_DIRTY 0)
  set(CMake_VERSION_IS_RELEASE 1)
  set(CMake_VERSION_SOURCE "")
else()
  set(CMake_VERSION_IS_DIRTY 0)
  set(CMake_VERSION_IS_RELEASE 0)
  # Try to identify the current development source version.
  set(CMake_VERSION_SOURCE "")
  if(EXISTS ${CMake_SOURCE_DIR}/.git/HEAD)
    find_program(GIT_EXECUTABLE NAMES git git.cmd)
    mark_as_advanced(GIT_EXECUTABLE)
    if(GIT_EXECUTABLE)
      execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --verify -q --short=4 HEAD
        OUTPUT_VARIABLE head
        OUTPUT_STRIP_TRAILING_WHITESPACE
        WORKING_DIRECTORY ${CMake_SOURCE_DIR}
        )
      if(head)
        set(CMake_VERSION_SOURCE "g${head}")
        execute_process(
          COMMAND ${GIT_EXECUTABLE} update-index -q --refresh
          WORKING_DIRECTORY ${CMake_SOURCE_DIR}
          )
        execute_process(
          COMMAND ${GIT_EXECUTABLE} diff-index --name-only HEAD --
          OUTPUT_VARIABLE dirty
          OUTPUT_STRIP_TRAILING_WHITESPACE
          WORKING_DIRECTORY ${CMake_SOURCE_DIR}
          )
        if(dirty)
          set(CMake_VERSION_IS_DIRTY 1)
        endif()
      endif()
    endif()
  endif()
endif()

# Compute the full version string.
set(CMake_VERSION ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}.${CMake_VERSION_PATCH})
if(CMake_VERSION_SOURCE)
  set(CMake_VERSION_SUFFIX "${CMake_VERSION_SOURCE}")
elseif(DEFINED CMake_VERSION_RC)
  set(CMake_VERSION_SUFFIX "rc${CMake_VERSION_RC}")
else()
  set(CMake_VERSION_SUFFIX "")
endif()
if(CMake_VERSION_SUFFIX)
  set(CMake_VERSION ${CMake_VERSION}-${CMake_VERSION_SUFFIX})
endif()
if(CMake_VERSION_IS_DIRTY)
  set(CMake_VERSION ${CMake_VERSION}-dirty)
endif()
